#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <regex>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>
using namespace std;

const uint32_t TEXT_START = 0x00400000, DATA_START = 0x10010000;
map<string,int> regno = {{"zero",0},{"at",1},{"v0",2},{"v1",3},{"a0",4},{"a1",5},{"a2",6},{"a3",7},{"t0",8},{"t1",9},{"t2",10},{"t3",11},{"t4",12},{"t5",13},{"t6",14},{"t7",15},{"s0",16},{"s1",17},{"s2",18},{"s3",19},{"s4",20},{"s5",21},{"s6",22},{"s7",23},{"t8",24},{"t9",25},{"k0",26},{"k1",27},{"gp",28},{"sp",29},{"fp",30},{"ra",31}};
map<string,int> funct = {{"add",0x20},{"addu",0x21},{"sub",0x22},{"div",0x1A},{"mfhi",0x10},{"syscall",0x0C}};
map<string,int> opcode = {{"addi",0x08},{"addiu",0x09},{"ori",0x0D},{"lui",0x0F},{"beq",0x04},{"bne",0x05}};
struct Entry { int line; string section, text; };
string trim(string s){ size_t a=s.find_first_not_of(" \t\r\n"), b=s.find_last_not_of(" \t\r\n"); return a==string::npos?"":s.substr(a,b-a+1); }
string comment(string s){ size_t p=s.find('#'); return trim(s.substr(0,p)); }
vector<string> split(string s){ vector<string> v; string x; stringstream ss(s); while(getline(ss,x,',')) if(!trim(x).empty()) v.push_back(trim(x)); return v; }
int regnum(string s){ s=trim(s); if(!s.empty()&&s[0]=='$')s=s.substr(1); if(!regno.count(s)) throw runtime_error("unknown register $"+s); return regno[s]; }
int64_t val(string s,const map<string,uint32_t>& sy){ s=trim(s); if(sy.count(s)) return sy.at(s); size_t p=0; long long n=stoll(s,&p,0); if(p!=s.size()) throw runtime_error("unknown symbol or number '"+s+"'"); return n; }
int64_t number(string s,const map<string,uint32_t>& sy){ s=trim(s); smatch m; if(regex_match(s,m,regex(R"(%hi\(([^)]+)\))"))) return (val(m[1],sy)>>16)&0xffff; if(regex_match(s,m,regex(R"(%lo\(([^)]+)\))"))) return val(m[1],sy)&0xffff; return val(s,sy); }
vector<uint8_t> stringBytes(string s){ s=trim(s); if(s.size()<2||s.front()!='"'||s.back()!='"') throw runtime_error(".asciiz requires a quoted string"); vector<uint8_t> b; for(size_t i=1;i+1<s.size();++i){ if(s[i]=='\\'&&i+1<s.size()-1){ char c=s[++i]; b.push_back(c=='n'?'\n':c=='t'?'\t':c=='r'?'\r':c=='"'?'"':c=='\\'?'\\':c); } else b.push_back((uint8_t)s[i]); } b.push_back(0); return b; }
bool ident(const string& s){ return regex_match(s,regex(R"([A-Za-z_][A-Za-z0-9_.]*)")); }
void write32(vector<uint8_t>& out,uint32_t w){ out.push_back(w>>24);out.push_back(w>>16);out.push_back(w>>8);out.push_back(w); }

struct Parsed { map<string,uint32_t> symbols; vector<Entry> entries; size_t textLen{},dataLen{}; };
Parsed firstPass(const vector<string>& lines){ Parsed p; string sec; uint32_t t=TEXT_START,d=DATA_START;
 for(size_t i=0;i<lines.size();++i){ string x=comment(lines[i]); if(x.empty())continue;
  while(true){ size_t c=x.find(':'); if(c==string::npos)break; string n=trim(x.substr(0,c)); if(!ident(n))break; if(p.symbols.count(n))throw runtime_error("line "+to_string(i+1)+": duplicate label '"+n+"'"); if(sec=="text")p.symbols[n]=t; else if(sec=="data")p.symbols[n]=d; else throw runtime_error("line "+to_string(i+1)+": label outside a section"); x=trim(x.substr(c+1)); if(x.empty())break; }
  if(x.empty())continue; if(x==".text"||x==".data"){sec=x.substr(1);p.entries.push_back({(int)i+1,sec,x});continue;} if(sec.empty())throw runtime_error("line "+to_string(i+1)+": outside a section"); p.entries.push_back({(int)i+1,sec,x});
  if(sec=="text"){ if(x[0]=='.')throw runtime_error("line "+to_string(i+1)+": unsupported text directive");t+=4;p.textLen+=4; }
  else { size_t sp=x.find(' ');string op=x.substr(0,sp), rest=sp==string::npos?"":trim(x.substr(sp+1)); if(op==".asciiz")d+=stringBytes(rest).size(),p.dataLen+=stringBytes(rest).size(); else if(op==".byte"){auto a=split(rest);if(a.empty())throw runtime_error("line "+to_string(i+1)+": .byte needs values");d+=a.size();p.dataLen+=a.size();} else throw runtime_error("line "+to_string(i+1)+": unsupported data directive '"+op+"'"); }
 } return p; }

uint32_t encode(const string& line,uint32_t pc,const map<string,uint32_t>& sy){ size_t sp=line.find(' ');string op=line.substr(0,sp),rest=sp==string::npos?"":trim(line.substr(sp+1));auto a=split(rest); if(funct.count(op)){int f=funct[op];if(op=="syscall"){if(!a.empty())throw runtime_error("syscall takes no operands");return f;}if(op=="mfhi"){if(a.size()!=1)throw runtime_error("mfhi needs rd");return regnum(a[0])<<11|f;}if(op=="div"){if(a.size()!=2)throw runtime_error("div needs rs, rt");return regnum(a[0])<<21|regnum(a[1])<<16|f;}if(a.size()!=3)throw runtime_error(op+" needs rd, rs, rt");return regnum(a[1])<<21|regnum(a[2])<<16|regnum(a[0])<<11|f;}
 if(opcode.count(op)){int rs=0,rt=0;int64_t imm=0;if(op=="lui"){if(a.size()!=2)throw runtime_error("lui needs rt, immediate");rt=regnum(a[0]);imm=number(a[1],sy);}else if(op=="beq"||op=="bne"){if(a.size()!=3)throw runtime_error(op+" needs rs, rt, label");rs=regnum(a[0]);rt=regnum(a[1]);int64_t target=val(a[2],sy),delta=target-(pc+4);if(delta%4)throw runtime_error("unaligned branch target");imm=delta/4;}else{if(a.size()!=3)throw runtime_error(op+" needs rt, rs, immediate");rt=regnum(a[0]);rs=regnum(a[1]);imm=number(a[2],sy);}if(imm<-32768||imm>65535)throw runtime_error("immediate out of range");return (opcode[op]<<26)|(rs<<21)|(rt<<16)|((uint16_t)imm);}
 if(op=="j"){if(a.size()!=1)throw runtime_error("j needs a target");uint32_t target=val(a[0],sy);if(target%4)throw runtime_error("unaligned jump target");return 2u<<26|(target>>2)&0x03ffffff;}throw runtime_error("unsupported instruction '"+op+"'"); }

int main(int argc,char**argv){try{if(argc<2){cerr<<"usage: assembler_cpp source [output_dir]\n";return 1;}string source=argv[1],outdir=argc>2?argv[2]:".";ifstream in(source);if(!in)throw runtime_error("cannot open source");vector<string> lines;string s;while(getline(in,s))lines.push_back(s);Parsed p=firstPass(lines);vector<uint8_t> text,data;uint32_t pc=TEXT_START;for(auto&e:p.entries){if(e.text==".text"||e.text==".data")continue;try{if(e.section=="text"){write32(text,encode(e.text,pc,p.symbols));pc+=4;}else{size_t sp=e.text.find(' ');string op=e.text.substr(0,sp),rest=sp==string::npos?"":trim(e.text.substr(sp+1));if(op==".asciiz"){auto b=stringBytes(rest);data.insert(data.end(),b.begin(),b.end());}else for(auto&x:split(rest)){auto n=number(x,p.symbols);if(n<0||n>255)throw runtime_error(".byte value out of range");data.push_back(n);}}}catch(exception&x){throw runtime_error("line "+to_string(e.line)+": "+x.what());}}
 filesystem::create_directories(outdir);ofstream ascii(filesystem::path(outdir)/"fizzbuzz.mhc-ascii");ascii<<"MHC-ASCII 1\nTEXT_START 0x00400000\nDATA_START 0x10010000\nTEXT_BYTES "<<text.size()<<"\nDATA_BYTES "<<data.size()<<"\nTEXT\n";for(size_t i=0;i<text.size();i+=4){for(size_t j=0;j<4;++j)ascii<<bitset<8>(text[i+j]);ascii<<'\n';}ascii<<"DATA\n";for(auto b:data)ascii<<bitset<8>(b)<<'\n';ofstream bin(filesystem::path(outdir)/"fizzbuzz.mhc-bin",ios::binary);bin.write("MHC1",4);uint32_t h[5]={1,TEXT_START,DATA_START,(uint32_t)text.size(),(uint32_t)data.size()};for(auto x:h){uint8_t q[4]={(uint8_t)(x>>24),(uint8_t)(x>>16),(uint8_t)(x>>8),(uint8_t)x};bin.write((char*)q,4);}bin.write((char*)text.data(),text.size());bin.write((char*)data.data(),data.size());cout<<"assembled "<<text.size()/4<<" instructions and "<<data.size()<<" data bytes\n";
 }catch(exception&e){cerr<<"assembler error: "<<e.what()<<'\n';return 1;} }
