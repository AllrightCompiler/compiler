
// Generated from frontend/SysY.g4 by ANTLR 4.10.1


#include "SysYVisitor.h"

#include "SysYParser.h"


using namespace antlrcpp;
using namespace frontend;

using namespace antlr4;

namespace {

struct SysYParserStaticData final {
  SysYParserStaticData(std::vector<std::string> ruleNames,
                        std::vector<std::string> literalNames,
                        std::vector<std::string> symbolicNames)
      : ruleNames(std::move(ruleNames)), literalNames(std::move(literalNames)),
        symbolicNames(std::move(symbolicNames)),
        vocabulary(this->literalNames, this->symbolicNames) {}

  SysYParserStaticData(const SysYParserStaticData&) = delete;
  SysYParserStaticData(SysYParserStaticData&&) = delete;
  SysYParserStaticData& operator=(const SysYParserStaticData&) = delete;
  SysYParserStaticData& operator=(SysYParserStaticData&&) = delete;

  std::vector<antlr4::dfa::DFA> decisionToDFA;
  antlr4::atn::PredictionContextCache sharedContextCache;
  const std::vector<std::string> ruleNames;
  const std::vector<std::string> literalNames;
  const std::vector<std::string> symbolicNames;
  const antlr4::dfa::Vocabulary vocabulary;
  antlr4::atn::SerializedATNView serializedATN;
  std::unique_ptr<antlr4::atn::ATN> atn;
};

std::once_flag sysyParserOnceFlag;
SysYParserStaticData *sysyParserStaticData = nullptr;

void sysyParserInitialize() {
  assert(sysyParserStaticData == nullptr);
  auto staticData = std::make_unique<SysYParserStaticData>(
    std::vector<std::string>{
      "compUnit", "compUnitItem", "decl", "constDecl", "bType", "constDef", 
      "varDecl", "varDef", "initVal", "funcDef", "funcType", "funcFParams", 
      "funcFParam", "block", "blockItem", "stmt", "exp", "cond", "lVal", 
      "primaryExp", "intConst", "floatConst", "number", "unaryExp", "stringConst", 
      "funcRParam", "funcRParams", "mulExp", "addExp", "relExp", "eqExp", 
      "lAndExp", "lOrExp"
    },
    std::vector<std::string>{
      "", "'int'", "'float'", "'void'", "'const'", "'if'", "'else'", "'while'", 
      "'break'", "'continue'", "'return'", "'='", "'+'", "'-'", "'*'", "'/'", 
      "'%'", "'=='", "'!='", "'<'", "'>'", "'<='", "'>='", "'!'", "'&&'", 
      "'||'", "','", "';'", "'('", "')'", "'['", "']'", "'{'", "'}'"
    },
    std::vector<std::string>{
      "", "Int", "Float", "Void", "Const", "If", "Else", "While", "Break", 
      "Continue", "Return", "Assign", "Add", "Sub", "Mul", "Div", "Mod", 
      "Eq", "Neq", "Lt", "Gt", "Leq", "Geq", "Not", "And", "Or", "Comma", 
      "Semicolon", "Lparen", "Rparen", "Lbracket", "Rbracket", "Lbrace", 
      "Rbrace", "Ident", "Whitespace", "LineComment", "BlockComment", "DecIntConst", 
      "OctIntConst", "HexIntConst", "DecFloatConst", "HexFloatConst", "StringConst"
    }
  );
  static const int32_t serializedATNSegment[] = {
  	4,1,43,391,2,0,7,0,2,1,7,1,2,2,7,2,2,3,7,3,2,4,7,4,2,5,7,5,2,6,7,6,2,
  	7,7,7,2,8,7,8,2,9,7,9,2,10,7,10,2,11,7,11,2,12,7,12,2,13,7,13,2,14,7,
  	14,2,15,7,15,2,16,7,16,2,17,7,17,2,18,7,18,2,19,7,19,2,20,7,20,2,21,7,
  	21,2,22,7,22,2,23,7,23,2,24,7,24,2,25,7,25,2,26,7,26,2,27,7,27,2,28,7,
  	28,2,29,7,29,2,30,7,30,2,31,7,31,2,32,7,32,1,0,5,0,68,8,0,10,0,12,0,71,
  	9,0,1,0,1,0,1,1,1,1,3,1,77,8,1,1,2,1,2,3,2,81,8,2,1,3,1,3,1,3,1,3,1,3,
  	5,3,88,8,3,10,3,12,3,91,9,3,1,3,1,3,1,4,1,4,3,4,97,8,4,1,5,1,5,1,5,1,
  	5,1,5,5,5,104,8,5,10,5,12,5,107,9,5,1,5,1,5,1,5,1,6,1,6,1,6,1,6,5,6,116,
  	8,6,10,6,12,6,119,9,6,1,6,1,6,1,7,1,7,1,7,1,7,1,7,5,7,128,8,7,10,7,12,
  	7,131,9,7,1,7,1,7,3,7,135,8,7,1,8,1,8,1,8,1,8,1,8,5,8,142,8,8,10,8,12,
  	8,145,9,8,3,8,147,8,8,1,8,3,8,150,8,8,1,9,1,9,1,9,1,9,3,9,156,8,9,1,9,
  	1,9,1,9,1,10,1,10,3,10,163,8,10,1,11,1,11,1,11,5,11,168,8,11,10,11,12,
  	11,171,9,11,1,12,1,12,1,12,1,12,1,12,1,12,1,12,1,12,1,12,1,12,1,12,5,
  	12,184,8,12,10,12,12,12,187,9,12,3,12,189,8,12,1,13,1,13,5,13,193,8,13,
  	10,13,12,13,196,9,13,1,13,1,13,1,14,1,14,3,14,202,8,14,1,15,1,15,1,15,
  	1,15,1,15,1,15,3,15,210,8,15,1,15,1,15,1,15,1,15,1,15,1,15,1,15,1,15,
  	1,15,3,15,221,8,15,1,15,1,15,1,15,1,15,1,15,1,15,1,15,1,15,1,15,1,15,
  	1,15,1,15,3,15,235,8,15,1,15,3,15,238,8,15,1,16,1,16,1,17,1,17,1,18,1,
  	18,1,18,1,18,1,18,5,18,249,8,18,10,18,12,18,252,9,18,1,19,1,19,1,19,1,
  	19,1,19,1,19,3,19,260,8,19,1,20,1,20,1,20,3,20,265,8,20,1,21,1,21,3,21,
  	269,8,21,1,22,1,22,3,22,273,8,22,1,23,1,23,1,23,1,23,3,23,279,8,23,1,
  	23,1,23,1,23,1,23,1,23,1,23,1,23,3,23,288,8,23,1,24,1,24,1,25,1,25,3,
  	25,294,8,25,1,26,1,26,1,26,5,26,299,8,26,10,26,12,26,302,9,26,1,27,1,
  	27,1,27,1,27,1,27,1,27,1,27,1,27,1,27,1,27,1,27,1,27,5,27,316,8,27,10,
  	27,12,27,319,9,27,1,28,1,28,1,28,1,28,1,28,1,28,1,28,1,28,1,28,5,28,330,
  	8,28,10,28,12,28,333,9,28,1,29,1,29,1,29,1,29,1,29,1,29,1,29,1,29,1,29,
  	1,29,1,29,1,29,1,29,1,29,1,29,5,29,350,8,29,10,29,12,29,353,9,29,1,30,
  	1,30,1,30,1,30,1,30,1,30,1,30,1,30,1,30,5,30,364,8,30,10,30,12,30,367,
  	9,30,1,31,1,31,1,31,1,31,1,31,1,31,5,31,375,8,31,10,31,12,31,378,9,31,
  	1,32,1,32,1,32,1,32,1,32,1,32,5,32,386,8,32,10,32,12,32,389,9,32,1,32,
  	0,6,54,56,58,60,62,64,33,0,2,4,6,8,10,12,14,16,18,20,22,24,26,28,30,32,
  	34,36,38,40,42,44,46,48,50,52,54,56,58,60,62,64,0,0,413,0,69,1,0,0,0,
  	2,76,1,0,0,0,4,80,1,0,0,0,6,82,1,0,0,0,8,96,1,0,0,0,10,98,1,0,0,0,12,
  	111,1,0,0,0,14,122,1,0,0,0,16,149,1,0,0,0,18,151,1,0,0,0,20,162,1,0,0,
  	0,22,164,1,0,0,0,24,188,1,0,0,0,26,190,1,0,0,0,28,201,1,0,0,0,30,237,
  	1,0,0,0,32,239,1,0,0,0,34,241,1,0,0,0,36,243,1,0,0,0,38,259,1,0,0,0,40,
  	264,1,0,0,0,42,268,1,0,0,0,44,272,1,0,0,0,46,287,1,0,0,0,48,289,1,0,0,
  	0,50,293,1,0,0,0,52,295,1,0,0,0,54,303,1,0,0,0,56,320,1,0,0,0,58,334,
  	1,0,0,0,60,354,1,0,0,0,62,368,1,0,0,0,64,379,1,0,0,0,66,68,3,2,1,0,67,
  	66,1,0,0,0,68,71,1,0,0,0,69,67,1,0,0,0,69,70,1,0,0,0,70,72,1,0,0,0,71,
  	69,1,0,0,0,72,73,5,0,0,1,73,1,1,0,0,0,74,77,3,4,2,0,75,77,3,18,9,0,76,
  	74,1,0,0,0,76,75,1,0,0,0,77,3,1,0,0,0,78,81,3,6,3,0,79,81,3,12,6,0,80,
  	78,1,0,0,0,80,79,1,0,0,0,81,5,1,0,0,0,82,83,5,4,0,0,83,84,3,8,4,0,84,
  	89,3,10,5,0,85,86,5,26,0,0,86,88,3,10,5,0,87,85,1,0,0,0,88,91,1,0,0,0,
  	89,87,1,0,0,0,89,90,1,0,0,0,90,92,1,0,0,0,91,89,1,0,0,0,92,93,5,27,0,
  	0,93,7,1,0,0,0,94,97,5,1,0,0,95,97,5,2,0,0,96,94,1,0,0,0,96,95,1,0,0,
  	0,97,9,1,0,0,0,98,105,5,34,0,0,99,100,5,30,0,0,100,101,3,32,16,0,101,
  	102,5,31,0,0,102,104,1,0,0,0,103,99,1,0,0,0,104,107,1,0,0,0,105,103,1,
  	0,0,0,105,106,1,0,0,0,106,108,1,0,0,0,107,105,1,0,0,0,108,109,5,11,0,
  	0,109,110,3,16,8,0,110,11,1,0,0,0,111,112,3,8,4,0,112,117,3,14,7,0,113,
  	114,5,26,0,0,114,116,3,14,7,0,115,113,1,0,0,0,116,119,1,0,0,0,117,115,
  	1,0,0,0,117,118,1,0,0,0,118,120,1,0,0,0,119,117,1,0,0,0,120,121,5,27,
  	0,0,121,13,1,0,0,0,122,129,5,34,0,0,123,124,5,30,0,0,124,125,3,32,16,
  	0,125,126,5,31,0,0,126,128,1,0,0,0,127,123,1,0,0,0,128,131,1,0,0,0,129,
  	127,1,0,0,0,129,130,1,0,0,0,130,134,1,0,0,0,131,129,1,0,0,0,132,133,5,
  	11,0,0,133,135,3,16,8,0,134,132,1,0,0,0,134,135,1,0,0,0,135,15,1,0,0,
  	0,136,150,3,32,16,0,137,146,5,32,0,0,138,143,3,16,8,0,139,140,5,26,0,
  	0,140,142,3,16,8,0,141,139,1,0,0,0,142,145,1,0,0,0,143,141,1,0,0,0,143,
  	144,1,0,0,0,144,147,1,0,0,0,145,143,1,0,0,0,146,138,1,0,0,0,146,147,1,
  	0,0,0,147,148,1,0,0,0,148,150,5,33,0,0,149,136,1,0,0,0,149,137,1,0,0,
  	0,150,17,1,0,0,0,151,152,3,20,10,0,152,153,5,34,0,0,153,155,5,28,0,0,
  	154,156,3,22,11,0,155,154,1,0,0,0,155,156,1,0,0,0,156,157,1,0,0,0,157,
  	158,5,29,0,0,158,159,3,26,13,0,159,19,1,0,0,0,160,163,3,8,4,0,161,163,
  	5,3,0,0,162,160,1,0,0,0,162,161,1,0,0,0,163,21,1,0,0,0,164,169,3,24,12,
  	0,165,166,5,26,0,0,166,168,3,24,12,0,167,165,1,0,0,0,168,171,1,0,0,0,
  	169,167,1,0,0,0,169,170,1,0,0,0,170,23,1,0,0,0,171,169,1,0,0,0,172,173,
  	3,8,4,0,173,174,5,34,0,0,174,189,1,0,0,0,175,176,3,8,4,0,176,177,5,34,
  	0,0,177,178,5,30,0,0,178,185,5,31,0,0,179,180,5,30,0,0,180,181,3,32,16,
  	0,181,182,5,31,0,0,182,184,1,0,0,0,183,179,1,0,0,0,184,187,1,0,0,0,185,
  	183,1,0,0,0,185,186,1,0,0,0,186,189,1,0,0,0,187,185,1,0,0,0,188,172,1,
  	0,0,0,188,175,1,0,0,0,189,25,1,0,0,0,190,194,5,32,0,0,191,193,3,28,14,
  	0,192,191,1,0,0,0,193,196,1,0,0,0,194,192,1,0,0,0,194,195,1,0,0,0,195,
  	197,1,0,0,0,196,194,1,0,0,0,197,198,5,33,0,0,198,27,1,0,0,0,199,202,3,
  	4,2,0,200,202,3,30,15,0,201,199,1,0,0,0,201,200,1,0,0,0,202,29,1,0,0,
  	0,203,204,3,36,18,0,204,205,5,11,0,0,205,206,3,32,16,0,206,207,5,27,0,
  	0,207,238,1,0,0,0,208,210,3,32,16,0,209,208,1,0,0,0,209,210,1,0,0,0,210,
  	211,1,0,0,0,211,238,5,27,0,0,212,238,3,26,13,0,213,214,5,5,0,0,214,215,
  	5,28,0,0,215,216,3,34,17,0,216,217,5,29,0,0,217,220,3,30,15,0,218,219,
  	5,6,0,0,219,221,3,30,15,0,220,218,1,0,0,0,220,221,1,0,0,0,221,238,1,0,
  	0,0,222,223,5,7,0,0,223,224,5,28,0,0,224,225,3,34,17,0,225,226,5,29,0,
  	0,226,227,3,30,15,0,227,238,1,0,0,0,228,229,5,8,0,0,229,238,5,27,0,0,
  	230,231,5,9,0,0,231,238,5,27,0,0,232,234,5,10,0,0,233,235,3,32,16,0,234,
  	233,1,0,0,0,234,235,1,0,0,0,235,236,1,0,0,0,236,238,5,27,0,0,237,203,
  	1,0,0,0,237,209,1,0,0,0,237,212,1,0,0,0,237,213,1,0,0,0,237,222,1,0,0,
  	0,237,228,1,0,0,0,237,230,1,0,0,0,237,232,1,0,0,0,238,31,1,0,0,0,239,
  	240,3,56,28,0,240,33,1,0,0,0,241,242,3,64,32,0,242,35,1,0,0,0,243,250,
  	5,34,0,0,244,245,5,30,0,0,245,246,3,32,16,0,246,247,5,31,0,0,247,249,
  	1,0,0,0,248,244,1,0,0,0,249,252,1,0,0,0,250,248,1,0,0,0,250,251,1,0,0,
  	0,251,37,1,0,0,0,252,250,1,0,0,0,253,254,5,28,0,0,254,255,3,32,16,0,255,
  	256,5,29,0,0,256,260,1,0,0,0,257,260,3,36,18,0,258,260,3,44,22,0,259,
  	253,1,0,0,0,259,257,1,0,0,0,259,258,1,0,0,0,260,39,1,0,0,0,261,265,5,
  	38,0,0,262,265,5,39,0,0,263,265,5,40,0,0,264,261,1,0,0,0,264,262,1,0,
  	0,0,264,263,1,0,0,0,265,41,1,0,0,0,266,269,5,41,0,0,267,269,5,42,0,0,
  	268,266,1,0,0,0,268,267,1,0,0,0,269,43,1,0,0,0,270,273,3,40,20,0,271,
  	273,3,42,21,0,272,270,1,0,0,0,272,271,1,0,0,0,273,45,1,0,0,0,274,288,
  	3,38,19,0,275,276,5,34,0,0,276,278,5,28,0,0,277,279,3,52,26,0,278,277,
  	1,0,0,0,278,279,1,0,0,0,279,280,1,0,0,0,280,288,5,29,0,0,281,282,5,12,
  	0,0,282,288,3,46,23,0,283,284,5,13,0,0,284,288,3,46,23,0,285,286,5,23,
  	0,0,286,288,3,46,23,0,287,274,1,0,0,0,287,275,1,0,0,0,287,281,1,0,0,0,
  	287,283,1,0,0,0,287,285,1,0,0,0,288,47,1,0,0,0,289,290,5,43,0,0,290,49,
  	1,0,0,0,291,294,3,32,16,0,292,294,3,48,24,0,293,291,1,0,0,0,293,292,1,
  	0,0,0,294,51,1,0,0,0,295,300,3,50,25,0,296,297,5,26,0,0,297,299,3,50,
  	25,0,298,296,1,0,0,0,299,302,1,0,0,0,300,298,1,0,0,0,300,301,1,0,0,0,
  	301,53,1,0,0,0,302,300,1,0,0,0,303,304,6,27,-1,0,304,305,3,46,23,0,305,
  	317,1,0,0,0,306,307,10,3,0,0,307,308,5,14,0,0,308,316,3,46,23,0,309,310,
  	10,2,0,0,310,311,5,15,0,0,311,316,3,46,23,0,312,313,10,1,0,0,313,314,
  	5,16,0,0,314,316,3,46,23,0,315,306,1,0,0,0,315,309,1,0,0,0,315,312,1,
  	0,0,0,316,319,1,0,0,0,317,315,1,0,0,0,317,318,1,0,0,0,318,55,1,0,0,0,
  	319,317,1,0,0,0,320,321,6,28,-1,0,321,322,3,54,27,0,322,331,1,0,0,0,323,
  	324,10,2,0,0,324,325,5,12,0,0,325,330,3,54,27,0,326,327,10,1,0,0,327,
  	328,5,13,0,0,328,330,3,54,27,0,329,323,1,0,0,0,329,326,1,0,0,0,330,333,
  	1,0,0,0,331,329,1,0,0,0,331,332,1,0,0,0,332,57,1,0,0,0,333,331,1,0,0,
  	0,334,335,6,29,-1,0,335,336,3,56,28,0,336,351,1,0,0,0,337,338,10,4,0,
  	0,338,339,5,19,0,0,339,350,3,56,28,0,340,341,10,3,0,0,341,342,5,20,0,
  	0,342,350,3,56,28,0,343,344,10,2,0,0,344,345,5,21,0,0,345,350,3,56,28,
  	0,346,347,10,1,0,0,347,348,5,22,0,0,348,350,3,56,28,0,349,337,1,0,0,0,
  	349,340,1,0,0,0,349,343,1,0,0,0,349,346,1,0,0,0,350,353,1,0,0,0,351,349,
  	1,0,0,0,351,352,1,0,0,0,352,59,1,0,0,0,353,351,1,0,0,0,354,355,6,30,-1,
  	0,355,356,3,58,29,0,356,365,1,0,0,0,357,358,10,2,0,0,358,359,5,17,0,0,
  	359,364,3,58,29,0,360,361,10,1,0,0,361,362,5,18,0,0,362,364,3,58,29,0,
  	363,357,1,0,0,0,363,360,1,0,0,0,364,367,1,0,0,0,365,363,1,0,0,0,365,366,
  	1,0,0,0,366,61,1,0,0,0,367,365,1,0,0,0,368,369,6,31,-1,0,369,370,3,60,
  	30,0,370,376,1,0,0,0,371,372,10,1,0,0,372,373,5,24,0,0,373,375,3,60,30,
  	0,374,371,1,0,0,0,375,378,1,0,0,0,376,374,1,0,0,0,376,377,1,0,0,0,377,
  	63,1,0,0,0,378,376,1,0,0,0,379,380,6,32,-1,0,380,381,3,62,31,0,381,387,
  	1,0,0,0,382,383,10,1,0,0,383,384,5,25,0,0,384,386,3,62,31,0,385,382,1,
  	0,0,0,386,389,1,0,0,0,387,385,1,0,0,0,387,388,1,0,0,0,388,65,1,0,0,0,
  	389,387,1,0,0,0,42,69,76,80,89,96,105,117,129,134,143,146,149,155,162,
  	169,185,188,194,201,209,220,234,237,250,259,264,268,272,278,287,293,300,
  	315,317,329,331,349,351,363,365,376,387
  };
  staticData->serializedATN = antlr4::atn::SerializedATNView(serializedATNSegment, sizeof(serializedATNSegment) / sizeof(serializedATNSegment[0]));

  antlr4::atn::ATNDeserializer deserializer;
  staticData->atn = deserializer.deserialize(staticData->serializedATN);

  const size_t count = staticData->atn->getNumberOfDecisions();
  staticData->decisionToDFA.reserve(count);
  for (size_t i = 0; i < count; i++) { 
    staticData->decisionToDFA.emplace_back(staticData->atn->getDecisionState(i), i);
  }
  sysyParserStaticData = staticData.release();
}

}

SysYParser::SysYParser(TokenStream *input) : SysYParser(input, antlr4::atn::ParserATNSimulatorOptions()) {}

SysYParser::SysYParser(TokenStream *input, const antlr4::atn::ParserATNSimulatorOptions &options) : Parser(input) {
  SysYParser::initialize();
  _interpreter = new atn::ParserATNSimulator(this, *sysyParserStaticData->atn, sysyParserStaticData->decisionToDFA, sysyParserStaticData->sharedContextCache, options);
}

SysYParser::~SysYParser() {
  delete _interpreter;
}

const atn::ATN& SysYParser::getATN() const {
  return *sysyParserStaticData->atn;
}

std::string SysYParser::getGrammarFileName() const {
  return "SysY.g4";
}

const std::vector<std::string>& SysYParser::getRuleNames() const {
  return sysyParserStaticData->ruleNames;
}

const dfa::Vocabulary& SysYParser::getVocabulary() const {
  return sysyParserStaticData->vocabulary;
}

antlr4::atn::SerializedATNView SysYParser::getSerializedATN() const {
  return sysyParserStaticData->serializedATN;
}


//----------------- CompUnitContext ------------------------------------------------------------------

SysYParser::CompUnitContext::CompUnitContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SysYParser::CompUnitContext::EOF() {
  return getToken(SysYParser::EOF, 0);
}

std::vector<SysYParser::CompUnitItemContext *> SysYParser::CompUnitContext::compUnitItem() {
  return getRuleContexts<SysYParser::CompUnitItemContext>();
}

SysYParser::CompUnitItemContext* SysYParser::CompUnitContext::compUnitItem(size_t i) {
  return getRuleContext<SysYParser::CompUnitItemContext>(i);
}


size_t SysYParser::CompUnitContext::getRuleIndex() const {
  return SysYParser::RuleCompUnit;
}


std::any SysYParser::CompUnitContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysYVisitor*>(visitor))
    return parserVisitor->visitCompUnit(this);
  else
    return visitor->visitChildren(this);
}

SysYParser::CompUnitContext* SysYParser::compUnit() {
  CompUnitContext *_localctx = _tracker.createInstance<CompUnitContext>(_ctx, getState());
  enterRule(_localctx, 0, SysYParser::RuleCompUnit);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(69);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while ((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & ((1ULL << SysYParser::Int)
      | (1ULL << SysYParser::Float)
      | (1ULL << SysYParser::Void)
      | (1ULL << SysYParser::Const))) != 0)) {
      setState(66);
      compUnitItem();
      setState(71);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
    setState(72);
    match(SysYParser::EOF);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- CompUnitItemContext ------------------------------------------------------------------

SysYParser::CompUnitItemContext::CompUnitItemContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

SysYParser::DeclContext* SysYParser::CompUnitItemContext::decl() {
  return getRuleContext<SysYParser::DeclContext>(0);
}

SysYParser::FuncDefContext* SysYParser::CompUnitItemContext::funcDef() {
  return getRuleContext<SysYParser::FuncDefContext>(0);
}


size_t SysYParser::CompUnitItemContext::getRuleIndex() const {
  return SysYParser::RuleCompUnitItem;
}


std::any SysYParser::CompUnitItemContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysYVisitor*>(visitor))
    return parserVisitor->visitCompUnitItem(this);
  else
    return visitor->visitChildren(this);
}

SysYParser::CompUnitItemContext* SysYParser::compUnitItem() {
  CompUnitItemContext *_localctx = _tracker.createInstance<CompUnitItemContext>(_ctx, getState());
  enterRule(_localctx, 2, SysYParser::RuleCompUnitItem);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(76);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 1, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(74);
      decl();
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(75);
      funcDef();
      break;
    }

    default:
      break;
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- DeclContext ------------------------------------------------------------------

SysYParser::DeclContext::DeclContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

SysYParser::ConstDeclContext* SysYParser::DeclContext::constDecl() {
  return getRuleContext<SysYParser::ConstDeclContext>(0);
}

SysYParser::VarDeclContext* SysYParser::DeclContext::varDecl() {
  return getRuleContext<SysYParser::VarDeclContext>(0);
}


size_t SysYParser::DeclContext::getRuleIndex() const {
  return SysYParser::RuleDecl;
}


std::any SysYParser::DeclContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysYVisitor*>(visitor))
    return parserVisitor->visitDecl(this);
  else
    return visitor->visitChildren(this);
}

SysYParser::DeclContext* SysYParser::decl() {
  DeclContext *_localctx = _tracker.createInstance<DeclContext>(_ctx, getState());
  enterRule(_localctx, 4, SysYParser::RuleDecl);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(80);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case SysYParser::Const: {
        enterOuterAlt(_localctx, 1);
        setState(78);
        constDecl();
        break;
      }

      case SysYParser::Int:
      case SysYParser::Float: {
        enterOuterAlt(_localctx, 2);
        setState(79);
        varDecl();
        break;
      }

    default:
      throw NoViableAltException(this);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ConstDeclContext ------------------------------------------------------------------

SysYParser::ConstDeclContext::ConstDeclContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SysYParser::ConstDeclContext::Const() {
  return getToken(SysYParser::Const, 0);
}

SysYParser::BTypeContext* SysYParser::ConstDeclContext::bType() {
  return getRuleContext<SysYParser::BTypeContext>(0);
}

std::vector<SysYParser::ConstDefContext *> SysYParser::ConstDeclContext::constDef() {
  return getRuleContexts<SysYParser::ConstDefContext>();
}

SysYParser::ConstDefContext* SysYParser::ConstDeclContext::constDef(size_t i) {
  return getRuleContext<SysYParser::ConstDefContext>(i);
}

tree::TerminalNode* SysYParser::ConstDeclContext::Semicolon() {
  return getToken(SysYParser::Semicolon, 0);
}

std::vector<tree::TerminalNode *> SysYParser::ConstDeclContext::Comma() {
  return getTokens(SysYParser::Comma);
}

tree::TerminalNode* SysYParser::ConstDeclContext::Comma(size_t i) {
  return getToken(SysYParser::Comma, i);
}


size_t SysYParser::ConstDeclContext::getRuleIndex() const {
  return SysYParser::RuleConstDecl;
}


std::any SysYParser::ConstDeclContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysYVisitor*>(visitor))
    return parserVisitor->visitConstDecl(this);
  else
    return visitor->visitChildren(this);
}

SysYParser::ConstDeclContext* SysYParser::constDecl() {
  ConstDeclContext *_localctx = _tracker.createInstance<ConstDeclContext>(_ctx, getState());
  enterRule(_localctx, 6, SysYParser::RuleConstDecl);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(82);
    match(SysYParser::Const);
    setState(83);
    bType();
    setState(84);
    constDef();
    setState(89);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == SysYParser::Comma) {
      setState(85);
      match(SysYParser::Comma);
      setState(86);
      constDef();
      setState(91);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
    setState(92);
    match(SysYParser::Semicolon);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- BTypeContext ------------------------------------------------------------------

SysYParser::BTypeContext::BTypeContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}


size_t SysYParser::BTypeContext::getRuleIndex() const {
  return SysYParser::RuleBType;
}

void SysYParser::BTypeContext::copyFrom(BTypeContext *ctx) {
  ParserRuleContext::copyFrom(ctx);
}

//----------------- FloatContext ------------------------------------------------------------------

tree::TerminalNode* SysYParser::FloatContext::Float() {
  return getToken(SysYParser::Float, 0);
}

SysYParser::FloatContext::FloatContext(BTypeContext *ctx) { copyFrom(ctx); }


std::any SysYParser::FloatContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysYVisitor*>(visitor))
    return parserVisitor->visitFloat(this);
  else
    return visitor->visitChildren(this);
}
//----------------- IntContext ------------------------------------------------------------------

tree::TerminalNode* SysYParser::IntContext::Int() {
  return getToken(SysYParser::Int, 0);
}

SysYParser::IntContext::IntContext(BTypeContext *ctx) { copyFrom(ctx); }


std::any SysYParser::IntContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysYVisitor*>(visitor))
    return parserVisitor->visitInt(this);
  else
    return visitor->visitChildren(this);
}
SysYParser::BTypeContext* SysYParser::bType() {
  BTypeContext *_localctx = _tracker.createInstance<BTypeContext>(_ctx, getState());
  enterRule(_localctx, 8, SysYParser::RuleBType);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(96);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case SysYParser::Int: {
        _localctx = _tracker.createInstance<SysYParser::IntContext>(_localctx);
        enterOuterAlt(_localctx, 1);
        setState(94);
        match(SysYParser::Int);
        break;
      }

      case SysYParser::Float: {
        _localctx = _tracker.createInstance<SysYParser::FloatContext>(_localctx);
        enterOuterAlt(_localctx, 2);
        setState(95);
        match(SysYParser::Float);
        break;
      }

    default:
      throw NoViableAltException(this);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ConstDefContext ------------------------------------------------------------------

SysYParser::ConstDefContext::ConstDefContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SysYParser::ConstDefContext::Ident() {
  return getToken(SysYParser::Ident, 0);
}

tree::TerminalNode* SysYParser::ConstDefContext::Assign() {
  return getToken(SysYParser::Assign, 0);
}

SysYParser::InitValContext* SysYParser::ConstDefContext::initVal() {
  return getRuleContext<SysYParser::InitValContext>(0);
}

std::vector<tree::TerminalNode *> SysYParser::ConstDefContext::Lbracket() {
  return getTokens(SysYParser::Lbracket);
}

tree::TerminalNode* SysYParser::ConstDefContext::Lbracket(size_t i) {
  return getToken(SysYParser::Lbracket, i);
}

std::vector<SysYParser::ExpContext *> SysYParser::ConstDefContext::exp() {
  return getRuleContexts<SysYParser::ExpContext>();
}

SysYParser::ExpContext* SysYParser::ConstDefContext::exp(size_t i) {
  return getRuleContext<SysYParser::ExpContext>(i);
}

std::vector<tree::TerminalNode *> SysYParser::ConstDefContext::Rbracket() {
  return getTokens(SysYParser::Rbracket);
}

tree::TerminalNode* SysYParser::ConstDefContext::Rbracket(size_t i) {
  return getToken(SysYParser::Rbracket, i);
}


size_t SysYParser::ConstDefContext::getRuleIndex() const {
  return SysYParser::RuleConstDef;
}


std::any SysYParser::ConstDefContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysYVisitor*>(visitor))
    return parserVisitor->visitConstDef(this);
  else
    return visitor->visitChildren(this);
}

SysYParser::ConstDefContext* SysYParser::constDef() {
  ConstDefContext *_localctx = _tracker.createInstance<ConstDefContext>(_ctx, getState());
  enterRule(_localctx, 10, SysYParser::RuleConstDef);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(98);
    match(SysYParser::Ident);
    setState(105);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == SysYParser::Lbracket) {
      setState(99);
      match(SysYParser::Lbracket);
      setState(100);
      exp();
      setState(101);
      match(SysYParser::Rbracket);
      setState(107);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
    setState(108);
    match(SysYParser::Assign);
    setState(109);
    initVal();
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- VarDeclContext ------------------------------------------------------------------

SysYParser::VarDeclContext::VarDeclContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

SysYParser::BTypeContext* SysYParser::VarDeclContext::bType() {
  return getRuleContext<SysYParser::BTypeContext>(0);
}

std::vector<SysYParser::VarDefContext *> SysYParser::VarDeclContext::varDef() {
  return getRuleContexts<SysYParser::VarDefContext>();
}

SysYParser::VarDefContext* SysYParser::VarDeclContext::varDef(size_t i) {
  return getRuleContext<SysYParser::VarDefContext>(i);
}

tree::TerminalNode* SysYParser::VarDeclContext::Semicolon() {
  return getToken(SysYParser::Semicolon, 0);
}

std::vector<tree::TerminalNode *> SysYParser::VarDeclContext::Comma() {
  return getTokens(SysYParser::Comma);
}

tree::TerminalNode* SysYParser::VarDeclContext::Comma(size_t i) {
  return getToken(SysYParser::Comma, i);
}


size_t SysYParser::VarDeclContext::getRuleIndex() const {
  return SysYParser::RuleVarDecl;
}


std::any SysYParser::VarDeclContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysYVisitor*>(visitor))
    return parserVisitor->visitVarDecl(this);
  else
    return visitor->visitChildren(this);
}

SysYParser::VarDeclContext* SysYParser::varDecl() {
  VarDeclContext *_localctx = _tracker.createInstance<VarDeclContext>(_ctx, getState());
  enterRule(_localctx, 12, SysYParser::RuleVarDecl);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(111);
    bType();
    setState(112);
    varDef();
    setState(117);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == SysYParser::Comma) {
      setState(113);
      match(SysYParser::Comma);
      setState(114);
      varDef();
      setState(119);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
    setState(120);
    match(SysYParser::Semicolon);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- VarDefContext ------------------------------------------------------------------

SysYParser::VarDefContext::VarDefContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SysYParser::VarDefContext::Ident() {
  return getToken(SysYParser::Ident, 0);
}

std::vector<tree::TerminalNode *> SysYParser::VarDefContext::Lbracket() {
  return getTokens(SysYParser::Lbracket);
}

tree::TerminalNode* SysYParser::VarDefContext::Lbracket(size_t i) {
  return getToken(SysYParser::Lbracket, i);
}

std::vector<SysYParser::ExpContext *> SysYParser::VarDefContext::exp() {
  return getRuleContexts<SysYParser::ExpContext>();
}

SysYParser::ExpContext* SysYParser::VarDefContext::exp(size_t i) {
  return getRuleContext<SysYParser::ExpContext>(i);
}

std::vector<tree::TerminalNode *> SysYParser::VarDefContext::Rbracket() {
  return getTokens(SysYParser::Rbracket);
}

tree::TerminalNode* SysYParser::VarDefContext::Rbracket(size_t i) {
  return getToken(SysYParser::Rbracket, i);
}

tree::TerminalNode* SysYParser::VarDefContext::Assign() {
  return getToken(SysYParser::Assign, 0);
}

SysYParser::InitValContext* SysYParser::VarDefContext::initVal() {
  return getRuleContext<SysYParser::InitValContext>(0);
}


size_t SysYParser::VarDefContext::getRuleIndex() const {
  return SysYParser::RuleVarDef;
}


std::any SysYParser::VarDefContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysYVisitor*>(visitor))
    return parserVisitor->visitVarDef(this);
  else
    return visitor->visitChildren(this);
}

SysYParser::VarDefContext* SysYParser::varDef() {
  VarDefContext *_localctx = _tracker.createInstance<VarDefContext>(_ctx, getState());
  enterRule(_localctx, 14, SysYParser::RuleVarDef);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(122);
    match(SysYParser::Ident);
    setState(129);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == SysYParser::Lbracket) {
      setState(123);
      match(SysYParser::Lbracket);
      setState(124);
      exp();
      setState(125);
      match(SysYParser::Rbracket);
      setState(131);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
    setState(134);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == SysYParser::Assign) {
      setState(132);
      match(SysYParser::Assign);
      setState(133);
      initVal();
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- InitValContext ------------------------------------------------------------------

SysYParser::InitValContext::InitValContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}


size_t SysYParser::InitValContext::getRuleIndex() const {
  return SysYParser::RuleInitVal;
}

void SysYParser::InitValContext::copyFrom(InitValContext *ctx) {
  ParserRuleContext::copyFrom(ctx);
}

//----------------- InitContext ------------------------------------------------------------------

SysYParser::ExpContext* SysYParser::InitContext::exp() {
  return getRuleContext<SysYParser::ExpContext>(0);
}

SysYParser::InitContext::InitContext(InitValContext *ctx) { copyFrom(ctx); }


std::any SysYParser::InitContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysYVisitor*>(visitor))
    return parserVisitor->visitInit(this);
  else
    return visitor->visitChildren(this);
}
//----------------- InitListContext ------------------------------------------------------------------

tree::TerminalNode* SysYParser::InitListContext::Lbrace() {
  return getToken(SysYParser::Lbrace, 0);
}

tree::TerminalNode* SysYParser::InitListContext::Rbrace() {
  return getToken(SysYParser::Rbrace, 0);
}

std::vector<SysYParser::InitValContext *> SysYParser::InitListContext::initVal() {
  return getRuleContexts<SysYParser::InitValContext>();
}

SysYParser::InitValContext* SysYParser::InitListContext::initVal(size_t i) {
  return getRuleContext<SysYParser::InitValContext>(i);
}

std::vector<tree::TerminalNode *> SysYParser::InitListContext::Comma() {
  return getTokens(SysYParser::Comma);
}

tree::TerminalNode* SysYParser::InitListContext::Comma(size_t i) {
  return getToken(SysYParser::Comma, i);
}

SysYParser::InitListContext::InitListContext(InitValContext *ctx) { copyFrom(ctx); }


std::any SysYParser::InitListContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysYVisitor*>(visitor))
    return parserVisitor->visitInitList(this);
  else
    return visitor->visitChildren(this);
}
SysYParser::InitValContext* SysYParser::initVal() {
  InitValContext *_localctx = _tracker.createInstance<InitValContext>(_ctx, getState());
  enterRule(_localctx, 16, SysYParser::RuleInitVal);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(149);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case SysYParser::Add:
      case SysYParser::Sub:
      case SysYParser::Not:
      case SysYParser::Lparen:
      case SysYParser::Ident:
      case SysYParser::DecIntConst:
      case SysYParser::OctIntConst:
      case SysYParser::HexIntConst:
      case SysYParser::DecFloatConst:
      case SysYParser::HexFloatConst: {
        _localctx = _tracker.createInstance<SysYParser::InitContext>(_localctx);
        enterOuterAlt(_localctx, 1);
        setState(136);
        exp();
        break;
      }

      case SysYParser::Lbrace: {
        _localctx = _tracker.createInstance<SysYParser::InitListContext>(_localctx);
        enterOuterAlt(_localctx, 2);
        setState(137);
        match(SysYParser::Lbrace);
        setState(146);
        _errHandler->sync(this);

        _la = _input->LA(1);
        if ((((_la & ~ 0x3fULL) == 0) &&
          ((1ULL << _la) & ((1ULL << SysYParser::Add)
          | (1ULL << SysYParser::Sub)
          | (1ULL << SysYParser::Not)
          | (1ULL << SysYParser::Lparen)
          | (1ULL << SysYParser::Lbrace)
          | (1ULL << SysYParser::Ident)
          | (1ULL << SysYParser::DecIntConst)
          | (1ULL << SysYParser::OctIntConst)
          | (1ULL << SysYParser::HexIntConst)
          | (1ULL << SysYParser::DecFloatConst)
          | (1ULL << SysYParser::HexFloatConst))) != 0)) {
          setState(138);
          initVal();
          setState(143);
          _errHandler->sync(this);
          _la = _input->LA(1);
          while (_la == SysYParser::Comma) {
            setState(139);
            match(SysYParser::Comma);
            setState(140);
            initVal();
            setState(145);
            _errHandler->sync(this);
            _la = _input->LA(1);
          }
        }
        setState(148);
        match(SysYParser::Rbrace);
        break;
      }

    default:
      throw NoViableAltException(this);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- FuncDefContext ------------------------------------------------------------------

SysYParser::FuncDefContext::FuncDefContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

SysYParser::FuncTypeContext* SysYParser::FuncDefContext::funcType() {
  return getRuleContext<SysYParser::FuncTypeContext>(0);
}

tree::TerminalNode* SysYParser::FuncDefContext::Ident() {
  return getToken(SysYParser::Ident, 0);
}

tree::TerminalNode* SysYParser::FuncDefContext::Lparen() {
  return getToken(SysYParser::Lparen, 0);
}

tree::TerminalNode* SysYParser::FuncDefContext::Rparen() {
  return getToken(SysYParser::Rparen, 0);
}

SysYParser::BlockContext* SysYParser::FuncDefContext::block() {
  return getRuleContext<SysYParser::BlockContext>(0);
}

SysYParser::FuncFParamsContext* SysYParser::FuncDefContext::funcFParams() {
  return getRuleContext<SysYParser::FuncFParamsContext>(0);
}


size_t SysYParser::FuncDefContext::getRuleIndex() const {
  return SysYParser::RuleFuncDef;
}


std::any SysYParser::FuncDefContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysYVisitor*>(visitor))
    return parserVisitor->visitFuncDef(this);
  else
    return visitor->visitChildren(this);
}

SysYParser::FuncDefContext* SysYParser::funcDef() {
  FuncDefContext *_localctx = _tracker.createInstance<FuncDefContext>(_ctx, getState());
  enterRule(_localctx, 18, SysYParser::RuleFuncDef);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(151);
    funcType();
    setState(152);
    match(SysYParser::Ident);
    setState(153);
    match(SysYParser::Lparen);
    setState(155);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == SysYParser::Int

    || _la == SysYParser::Float) {
      setState(154);
      funcFParams();
    }
    setState(157);
    match(SysYParser::Rparen);
    setState(158);
    block();
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- FuncTypeContext ------------------------------------------------------------------

SysYParser::FuncTypeContext::FuncTypeContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}


size_t SysYParser::FuncTypeContext::getRuleIndex() const {
  return SysYParser::RuleFuncType;
}

void SysYParser::FuncTypeContext::copyFrom(FuncTypeContext *ctx) {
  ParserRuleContext::copyFrom(ctx);
}

//----------------- FuncType_Context ------------------------------------------------------------------

SysYParser::BTypeContext* SysYParser::FuncType_Context::bType() {
  return getRuleContext<SysYParser::BTypeContext>(0);
}

SysYParser::FuncType_Context::FuncType_Context(FuncTypeContext *ctx) { copyFrom(ctx); }


std::any SysYParser::FuncType_Context::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysYVisitor*>(visitor))
    return parserVisitor->visitFuncType_(this);
  else
    return visitor->visitChildren(this);
}
//----------------- VoidContext ------------------------------------------------------------------

tree::TerminalNode* SysYParser::VoidContext::Void() {
  return getToken(SysYParser::Void, 0);
}

SysYParser::VoidContext::VoidContext(FuncTypeContext *ctx) { copyFrom(ctx); }


std::any SysYParser::VoidContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysYVisitor*>(visitor))
    return parserVisitor->visitVoid(this);
  else
    return visitor->visitChildren(this);
}
SysYParser::FuncTypeContext* SysYParser::funcType() {
  FuncTypeContext *_localctx = _tracker.createInstance<FuncTypeContext>(_ctx, getState());
  enterRule(_localctx, 20, SysYParser::RuleFuncType);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(162);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case SysYParser::Int:
      case SysYParser::Float: {
        _localctx = _tracker.createInstance<SysYParser::FuncType_Context>(_localctx);
        enterOuterAlt(_localctx, 1);
        setState(160);
        bType();
        break;
      }

      case SysYParser::Void: {
        _localctx = _tracker.createInstance<SysYParser::VoidContext>(_localctx);
        enterOuterAlt(_localctx, 2);
        setState(161);
        match(SysYParser::Void);
        break;
      }

    default:
      throw NoViableAltException(this);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- FuncFParamsContext ------------------------------------------------------------------

SysYParser::FuncFParamsContext::FuncFParamsContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<SysYParser::FuncFParamContext *> SysYParser::FuncFParamsContext::funcFParam() {
  return getRuleContexts<SysYParser::FuncFParamContext>();
}

SysYParser::FuncFParamContext* SysYParser::FuncFParamsContext::funcFParam(size_t i) {
  return getRuleContext<SysYParser::FuncFParamContext>(i);
}

std::vector<tree::TerminalNode *> SysYParser::FuncFParamsContext::Comma() {
  return getTokens(SysYParser::Comma);
}

tree::TerminalNode* SysYParser::FuncFParamsContext::Comma(size_t i) {
  return getToken(SysYParser::Comma, i);
}


size_t SysYParser::FuncFParamsContext::getRuleIndex() const {
  return SysYParser::RuleFuncFParams;
}


std::any SysYParser::FuncFParamsContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysYVisitor*>(visitor))
    return parserVisitor->visitFuncFParams(this);
  else
    return visitor->visitChildren(this);
}

SysYParser::FuncFParamsContext* SysYParser::funcFParams() {
  FuncFParamsContext *_localctx = _tracker.createInstance<FuncFParamsContext>(_ctx, getState());
  enterRule(_localctx, 22, SysYParser::RuleFuncFParams);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(164);
    funcFParam();
    setState(169);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == SysYParser::Comma) {
      setState(165);
      match(SysYParser::Comma);
      setState(166);
      funcFParam();
      setState(171);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- FuncFParamContext ------------------------------------------------------------------

SysYParser::FuncFParamContext::FuncFParamContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}


size_t SysYParser::FuncFParamContext::getRuleIndex() const {
  return SysYParser::RuleFuncFParam;
}

void SysYParser::FuncFParamContext::copyFrom(FuncFParamContext *ctx) {
  ParserRuleContext::copyFrom(ctx);
}

//----------------- ScalarParamContext ------------------------------------------------------------------

SysYParser::BTypeContext* SysYParser::ScalarParamContext::bType() {
  return getRuleContext<SysYParser::BTypeContext>(0);
}

tree::TerminalNode* SysYParser::ScalarParamContext::Ident() {
  return getToken(SysYParser::Ident, 0);
}

SysYParser::ScalarParamContext::ScalarParamContext(FuncFParamContext *ctx) { copyFrom(ctx); }


std::any SysYParser::ScalarParamContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysYVisitor*>(visitor))
    return parserVisitor->visitScalarParam(this);
  else
    return visitor->visitChildren(this);
}
//----------------- ArrayParamContext ------------------------------------------------------------------

SysYParser::BTypeContext* SysYParser::ArrayParamContext::bType() {
  return getRuleContext<SysYParser::BTypeContext>(0);
}

tree::TerminalNode* SysYParser::ArrayParamContext::Ident() {
  return getToken(SysYParser::Ident, 0);
}

std::vector<tree::TerminalNode *> SysYParser::ArrayParamContext::Lbracket() {
  return getTokens(SysYParser::Lbracket);
}

tree::TerminalNode* SysYParser::ArrayParamContext::Lbracket(size_t i) {
  return getToken(SysYParser::Lbracket, i);
}

std::vector<tree::TerminalNode *> SysYParser::ArrayParamContext::Rbracket() {
  return getTokens(SysYParser::Rbracket);
}

tree::TerminalNode* SysYParser::ArrayParamContext::Rbracket(size_t i) {
  return getToken(SysYParser::Rbracket, i);
}

std::vector<SysYParser::ExpContext *> SysYParser::ArrayParamContext::exp() {
  return getRuleContexts<SysYParser::ExpContext>();
}

SysYParser::ExpContext* SysYParser::ArrayParamContext::exp(size_t i) {
  return getRuleContext<SysYParser::ExpContext>(i);
}

SysYParser::ArrayParamContext::ArrayParamContext(FuncFParamContext *ctx) { copyFrom(ctx); }


std::any SysYParser::ArrayParamContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysYVisitor*>(visitor))
    return parserVisitor->visitArrayParam(this);
  else
    return visitor->visitChildren(this);
}
SysYParser::FuncFParamContext* SysYParser::funcFParam() {
  FuncFParamContext *_localctx = _tracker.createInstance<FuncFParamContext>(_ctx, getState());
  enterRule(_localctx, 24, SysYParser::RuleFuncFParam);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(188);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 16, _ctx)) {
    case 1: {
      _localctx = _tracker.createInstance<SysYParser::ScalarParamContext>(_localctx);
      enterOuterAlt(_localctx, 1);
      setState(172);
      bType();
      setState(173);
      match(SysYParser::Ident);
      break;
    }

    case 2: {
      _localctx = _tracker.createInstance<SysYParser::ArrayParamContext>(_localctx);
      enterOuterAlt(_localctx, 2);
      setState(175);
      bType();
      setState(176);
      match(SysYParser::Ident);
      setState(177);
      match(SysYParser::Lbracket);
      setState(178);
      match(SysYParser::Rbracket);
      setState(185);
      _errHandler->sync(this);
      _la = _input->LA(1);
      while (_la == SysYParser::Lbracket) {
        setState(179);
        match(SysYParser::Lbracket);
        setState(180);
        exp();
        setState(181);
        match(SysYParser::Rbracket);
        setState(187);
        _errHandler->sync(this);
        _la = _input->LA(1);
      }
      break;
    }

    default:
      break;
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- BlockContext ------------------------------------------------------------------

SysYParser::BlockContext::BlockContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SysYParser::BlockContext::Lbrace() {
  return getToken(SysYParser::Lbrace, 0);
}

tree::TerminalNode* SysYParser::BlockContext::Rbrace() {
  return getToken(SysYParser::Rbrace, 0);
}

std::vector<SysYParser::BlockItemContext *> SysYParser::BlockContext::blockItem() {
  return getRuleContexts<SysYParser::BlockItemContext>();
}

SysYParser::BlockItemContext* SysYParser::BlockContext::blockItem(size_t i) {
  return getRuleContext<SysYParser::BlockItemContext>(i);
}


size_t SysYParser::BlockContext::getRuleIndex() const {
  return SysYParser::RuleBlock;
}


std::any SysYParser::BlockContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysYVisitor*>(visitor))
    return parserVisitor->visitBlock(this);
  else
    return visitor->visitChildren(this);
}

SysYParser::BlockContext* SysYParser::block() {
  BlockContext *_localctx = _tracker.createInstance<BlockContext>(_ctx, getState());
  enterRule(_localctx, 26, SysYParser::RuleBlock);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(190);
    match(SysYParser::Lbrace);
    setState(194);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while ((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & ((1ULL << SysYParser::Int)
      | (1ULL << SysYParser::Float)
      | (1ULL << SysYParser::Const)
      | (1ULL << SysYParser::If)
      | (1ULL << SysYParser::While)
      | (1ULL << SysYParser::Break)
      | (1ULL << SysYParser::Continue)
      | (1ULL << SysYParser::Return)
      | (1ULL << SysYParser::Add)
      | (1ULL << SysYParser::Sub)
      | (1ULL << SysYParser::Not)
      | (1ULL << SysYParser::Semicolon)
      | (1ULL << SysYParser::Lparen)
      | (1ULL << SysYParser::Lbrace)
      | (1ULL << SysYParser::Ident)
      | (1ULL << SysYParser::DecIntConst)
      | (1ULL << SysYParser::OctIntConst)
      | (1ULL << SysYParser::HexIntConst)
      | (1ULL << SysYParser::DecFloatConst)
      | (1ULL << SysYParser::HexFloatConst))) != 0)) {
      setState(191);
      blockItem();
      setState(196);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
    setState(197);
    match(SysYParser::Rbrace);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- BlockItemContext ------------------------------------------------------------------

SysYParser::BlockItemContext::BlockItemContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

SysYParser::DeclContext* SysYParser::BlockItemContext::decl() {
  return getRuleContext<SysYParser::DeclContext>(0);
}

SysYParser::StmtContext* SysYParser::BlockItemContext::stmt() {
  return getRuleContext<SysYParser::StmtContext>(0);
}


size_t SysYParser::BlockItemContext::getRuleIndex() const {
  return SysYParser::RuleBlockItem;
}


std::any SysYParser::BlockItemContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysYVisitor*>(visitor))
    return parserVisitor->visitBlockItem(this);
  else
    return visitor->visitChildren(this);
}

SysYParser::BlockItemContext* SysYParser::blockItem() {
  BlockItemContext *_localctx = _tracker.createInstance<BlockItemContext>(_ctx, getState());
  enterRule(_localctx, 28, SysYParser::RuleBlockItem);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(201);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case SysYParser::Int:
      case SysYParser::Float:
      case SysYParser::Const: {
        enterOuterAlt(_localctx, 1);
        setState(199);
        decl();
        break;
      }

      case SysYParser::If:
      case SysYParser::While:
      case SysYParser::Break:
      case SysYParser::Continue:
      case SysYParser::Return:
      case SysYParser::Add:
      case SysYParser::Sub:
      case SysYParser::Not:
      case SysYParser::Semicolon:
      case SysYParser::Lparen:
      case SysYParser::Lbrace:
      case SysYParser::Ident:
      case SysYParser::DecIntConst:
      case SysYParser::OctIntConst:
      case SysYParser::HexIntConst:
      case SysYParser::DecFloatConst:
      case SysYParser::HexFloatConst: {
        enterOuterAlt(_localctx, 2);
        setState(200);
        stmt();
        break;
      }

    default:
      throw NoViableAltException(this);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- StmtContext ------------------------------------------------------------------

SysYParser::StmtContext::StmtContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}


size_t SysYParser::StmtContext::getRuleIndex() const {
  return SysYParser::RuleStmt;
}

void SysYParser::StmtContext::copyFrom(StmtContext *ctx) {
  ParserRuleContext::copyFrom(ctx);
}

//----------------- ExprStmtContext ------------------------------------------------------------------

tree::TerminalNode* SysYParser::ExprStmtContext::Semicolon() {
  return getToken(SysYParser::Semicolon, 0);
}

SysYParser::ExpContext* SysYParser::ExprStmtContext::exp() {
  return getRuleContext<SysYParser::ExpContext>(0);
}

SysYParser::ExprStmtContext::ExprStmtContext(StmtContext *ctx) { copyFrom(ctx); }


std::any SysYParser::ExprStmtContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysYVisitor*>(visitor))
    return parserVisitor->visitExprStmt(this);
  else
    return visitor->visitChildren(this);
}
//----------------- BlockStmtContext ------------------------------------------------------------------

SysYParser::BlockContext* SysYParser::BlockStmtContext::block() {
  return getRuleContext<SysYParser::BlockContext>(0);
}

SysYParser::BlockStmtContext::BlockStmtContext(StmtContext *ctx) { copyFrom(ctx); }


std::any SysYParser::BlockStmtContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysYVisitor*>(visitor))
    return parserVisitor->visitBlockStmt(this);
  else
    return visitor->visitChildren(this);
}
//----------------- BreakContext ------------------------------------------------------------------

tree::TerminalNode* SysYParser::BreakContext::Break() {
  return getToken(SysYParser::Break, 0);
}

tree::TerminalNode* SysYParser::BreakContext::Semicolon() {
  return getToken(SysYParser::Semicolon, 0);
}

SysYParser::BreakContext::BreakContext(StmtContext *ctx) { copyFrom(ctx); }


std::any SysYParser::BreakContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysYVisitor*>(visitor))
    return parserVisitor->visitBreak(this);
  else
    return visitor->visitChildren(this);
}
//----------------- ContinueContext ------------------------------------------------------------------

tree::TerminalNode* SysYParser::ContinueContext::Continue() {
  return getToken(SysYParser::Continue, 0);
}

tree::TerminalNode* SysYParser::ContinueContext::Semicolon() {
  return getToken(SysYParser::Semicolon, 0);
}

SysYParser::ContinueContext::ContinueContext(StmtContext *ctx) { copyFrom(ctx); }


std::any SysYParser::ContinueContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysYVisitor*>(visitor))
    return parserVisitor->visitContinue(this);
  else
    return visitor->visitChildren(this);
}
//----------------- WhileContext ------------------------------------------------------------------

tree::TerminalNode* SysYParser::WhileContext::While() {
  return getToken(SysYParser::While, 0);
}

tree::TerminalNode* SysYParser::WhileContext::Lparen() {
  return getToken(SysYParser::Lparen, 0);
}

SysYParser::CondContext* SysYParser::WhileContext::cond() {
  return getRuleContext<SysYParser::CondContext>(0);
}

tree::TerminalNode* SysYParser::WhileContext::Rparen() {
  return getToken(SysYParser::Rparen, 0);
}

SysYParser::StmtContext* SysYParser::WhileContext::stmt() {
  return getRuleContext<SysYParser::StmtContext>(0);
}

SysYParser::WhileContext::WhileContext(StmtContext *ctx) { copyFrom(ctx); }


std::any SysYParser::WhileContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysYVisitor*>(visitor))
    return parserVisitor->visitWhile(this);
  else
    return visitor->visitChildren(this);
}
//----------------- IfElseContext ------------------------------------------------------------------

tree::TerminalNode* SysYParser::IfElseContext::If() {
  return getToken(SysYParser::If, 0);
}

tree::TerminalNode* SysYParser::IfElseContext::Lparen() {
  return getToken(SysYParser::Lparen, 0);
}

SysYParser::CondContext* SysYParser::IfElseContext::cond() {
  return getRuleContext<SysYParser::CondContext>(0);
}

tree::TerminalNode* SysYParser::IfElseContext::Rparen() {
  return getToken(SysYParser::Rparen, 0);
}

std::vector<SysYParser::StmtContext *> SysYParser::IfElseContext::stmt() {
  return getRuleContexts<SysYParser::StmtContext>();
}

SysYParser::StmtContext* SysYParser::IfElseContext::stmt(size_t i) {
  return getRuleContext<SysYParser::StmtContext>(i);
}

tree::TerminalNode* SysYParser::IfElseContext::Else() {
  return getToken(SysYParser::Else, 0);
}

SysYParser::IfElseContext::IfElseContext(StmtContext *ctx) { copyFrom(ctx); }


std::any SysYParser::IfElseContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysYVisitor*>(visitor))
    return parserVisitor->visitIfElse(this);
  else
    return visitor->visitChildren(this);
}
//----------------- ReturnContext ------------------------------------------------------------------

tree::TerminalNode* SysYParser::ReturnContext::Return() {
  return getToken(SysYParser::Return, 0);
}

tree::TerminalNode* SysYParser::ReturnContext::Semicolon() {
  return getToken(SysYParser::Semicolon, 0);
}

SysYParser::ExpContext* SysYParser::ReturnContext::exp() {
  return getRuleContext<SysYParser::ExpContext>(0);
}

SysYParser::ReturnContext::ReturnContext(StmtContext *ctx) { copyFrom(ctx); }


std::any SysYParser::ReturnContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysYVisitor*>(visitor))
    return parserVisitor->visitReturn(this);
  else
    return visitor->visitChildren(this);
}
//----------------- AssignContext ------------------------------------------------------------------

SysYParser::LValContext* SysYParser::AssignContext::lVal() {
  return getRuleContext<SysYParser::LValContext>(0);
}

tree::TerminalNode* SysYParser::AssignContext::Assign() {
  return getToken(SysYParser::Assign, 0);
}

SysYParser::ExpContext* SysYParser::AssignContext::exp() {
  return getRuleContext<SysYParser::ExpContext>(0);
}

tree::TerminalNode* SysYParser::AssignContext::Semicolon() {
  return getToken(SysYParser::Semicolon, 0);
}

SysYParser::AssignContext::AssignContext(StmtContext *ctx) { copyFrom(ctx); }


std::any SysYParser::AssignContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysYVisitor*>(visitor))
    return parserVisitor->visitAssign(this);
  else
    return visitor->visitChildren(this);
}
SysYParser::StmtContext* SysYParser::stmt() {
  StmtContext *_localctx = _tracker.createInstance<StmtContext>(_ctx, getState());
  enterRule(_localctx, 30, SysYParser::RuleStmt);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(237);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 22, _ctx)) {
    case 1: {
      _localctx = _tracker.createInstance<SysYParser::AssignContext>(_localctx);
      enterOuterAlt(_localctx, 1);
      setState(203);
      lVal();
      setState(204);
      match(SysYParser::Assign);
      setState(205);
      exp();
      setState(206);
      match(SysYParser::Semicolon);
      break;
    }

    case 2: {
      _localctx = _tracker.createInstance<SysYParser::ExprStmtContext>(_localctx);
      enterOuterAlt(_localctx, 2);
      setState(209);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if ((((_la & ~ 0x3fULL) == 0) &&
        ((1ULL << _la) & ((1ULL << SysYParser::Add)
        | (1ULL << SysYParser::Sub)
        | (1ULL << SysYParser::Not)
        | (1ULL << SysYParser::Lparen)
        | (1ULL << SysYParser::Ident)
        | (1ULL << SysYParser::DecIntConst)
        | (1ULL << SysYParser::OctIntConst)
        | (1ULL << SysYParser::HexIntConst)
        | (1ULL << SysYParser::DecFloatConst)
        | (1ULL << SysYParser::HexFloatConst))) != 0)) {
        setState(208);
        exp();
      }
      setState(211);
      match(SysYParser::Semicolon);
      break;
    }

    case 3: {
      _localctx = _tracker.createInstance<SysYParser::BlockStmtContext>(_localctx);
      enterOuterAlt(_localctx, 3);
      setState(212);
      block();
      break;
    }

    case 4: {
      _localctx = _tracker.createInstance<SysYParser::IfElseContext>(_localctx);
      enterOuterAlt(_localctx, 4);
      setState(213);
      match(SysYParser::If);
      setState(214);
      match(SysYParser::Lparen);
      setState(215);
      cond();
      setState(216);
      match(SysYParser::Rparen);
      setState(217);
      stmt();
      setState(220);
      _errHandler->sync(this);

      switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 20, _ctx)) {
      case 1: {
        setState(218);
        match(SysYParser::Else);
        setState(219);
        stmt();
        break;
      }

      default:
        break;
      }
      break;
    }

    case 5: {
      _localctx = _tracker.createInstance<SysYParser::WhileContext>(_localctx);
      enterOuterAlt(_localctx, 5);
      setState(222);
      match(SysYParser::While);
      setState(223);
      match(SysYParser::Lparen);
      setState(224);
      cond();
      setState(225);
      match(SysYParser::Rparen);
      setState(226);
      stmt();
      break;
    }

    case 6: {
      _localctx = _tracker.createInstance<SysYParser::BreakContext>(_localctx);
      enterOuterAlt(_localctx, 6);
      setState(228);
      match(SysYParser::Break);
      setState(229);
      match(SysYParser::Semicolon);
      break;
    }

    case 7: {
      _localctx = _tracker.createInstance<SysYParser::ContinueContext>(_localctx);
      enterOuterAlt(_localctx, 7);
      setState(230);
      match(SysYParser::Continue);
      setState(231);
      match(SysYParser::Semicolon);
      break;
    }

    case 8: {
      _localctx = _tracker.createInstance<SysYParser::ReturnContext>(_localctx);
      enterOuterAlt(_localctx, 8);
      setState(232);
      match(SysYParser::Return);
      setState(234);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if ((((_la & ~ 0x3fULL) == 0) &&
        ((1ULL << _la) & ((1ULL << SysYParser::Add)
        | (1ULL << SysYParser::Sub)
        | (1ULL << SysYParser::Not)
        | (1ULL << SysYParser::Lparen)
        | (1ULL << SysYParser::Ident)
        | (1ULL << SysYParser::DecIntConst)
        | (1ULL << SysYParser::OctIntConst)
        | (1ULL << SysYParser::HexIntConst)
        | (1ULL << SysYParser::DecFloatConst)
        | (1ULL << SysYParser::HexFloatConst))) != 0)) {
        setState(233);
        exp();
      }
      setState(236);
      match(SysYParser::Semicolon);
      break;
    }

    default:
      break;
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ExpContext ------------------------------------------------------------------

SysYParser::ExpContext::ExpContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

SysYParser::AddExpContext* SysYParser::ExpContext::addExp() {
  return getRuleContext<SysYParser::AddExpContext>(0);
}


size_t SysYParser::ExpContext::getRuleIndex() const {
  return SysYParser::RuleExp;
}


std::any SysYParser::ExpContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysYVisitor*>(visitor))
    return parserVisitor->visitExp(this);
  else
    return visitor->visitChildren(this);
}

SysYParser::ExpContext* SysYParser::exp() {
  ExpContext *_localctx = _tracker.createInstance<ExpContext>(_ctx, getState());
  enterRule(_localctx, 32, SysYParser::RuleExp);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(239);
    addExp(0);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- CondContext ------------------------------------------------------------------

SysYParser::CondContext::CondContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

SysYParser::LOrExpContext* SysYParser::CondContext::lOrExp() {
  return getRuleContext<SysYParser::LOrExpContext>(0);
}


size_t SysYParser::CondContext::getRuleIndex() const {
  return SysYParser::RuleCond;
}


std::any SysYParser::CondContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysYVisitor*>(visitor))
    return parserVisitor->visitCond(this);
  else
    return visitor->visitChildren(this);
}

SysYParser::CondContext* SysYParser::cond() {
  CondContext *_localctx = _tracker.createInstance<CondContext>(_ctx, getState());
  enterRule(_localctx, 34, SysYParser::RuleCond);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(241);
    lOrExp(0);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- LValContext ------------------------------------------------------------------

SysYParser::LValContext::LValContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SysYParser::LValContext::Ident() {
  return getToken(SysYParser::Ident, 0);
}

std::vector<tree::TerminalNode *> SysYParser::LValContext::Lbracket() {
  return getTokens(SysYParser::Lbracket);
}

tree::TerminalNode* SysYParser::LValContext::Lbracket(size_t i) {
  return getToken(SysYParser::Lbracket, i);
}

std::vector<SysYParser::ExpContext *> SysYParser::LValContext::exp() {
  return getRuleContexts<SysYParser::ExpContext>();
}

SysYParser::ExpContext* SysYParser::LValContext::exp(size_t i) {
  return getRuleContext<SysYParser::ExpContext>(i);
}

std::vector<tree::TerminalNode *> SysYParser::LValContext::Rbracket() {
  return getTokens(SysYParser::Rbracket);
}

tree::TerminalNode* SysYParser::LValContext::Rbracket(size_t i) {
  return getToken(SysYParser::Rbracket, i);
}


size_t SysYParser::LValContext::getRuleIndex() const {
  return SysYParser::RuleLVal;
}


std::any SysYParser::LValContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysYVisitor*>(visitor))
    return parserVisitor->visitLVal(this);
  else
    return visitor->visitChildren(this);
}

SysYParser::LValContext* SysYParser::lVal() {
  LValContext *_localctx = _tracker.createInstance<LValContext>(_ctx, getState());
  enterRule(_localctx, 36, SysYParser::RuleLVal);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    size_t alt;
    enterOuterAlt(_localctx, 1);
    setState(243);
    match(SysYParser::Ident);
    setState(250);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 23, _ctx);
    while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1) {
        setState(244);
        match(SysYParser::Lbracket);
        setState(245);
        exp();
        setState(246);
        match(SysYParser::Rbracket); 
      }
      setState(252);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 23, _ctx);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- PrimaryExpContext ------------------------------------------------------------------

SysYParser::PrimaryExpContext::PrimaryExpContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}


size_t SysYParser::PrimaryExpContext::getRuleIndex() const {
  return SysYParser::RulePrimaryExp;
}

void SysYParser::PrimaryExpContext::copyFrom(PrimaryExpContext *ctx) {
  ParserRuleContext::copyFrom(ctx);
}

//----------------- PrimaryExp_Context ------------------------------------------------------------------

tree::TerminalNode* SysYParser::PrimaryExp_Context::Lparen() {
  return getToken(SysYParser::Lparen, 0);
}

SysYParser::ExpContext* SysYParser::PrimaryExp_Context::exp() {
  return getRuleContext<SysYParser::ExpContext>(0);
}

tree::TerminalNode* SysYParser::PrimaryExp_Context::Rparen() {
  return getToken(SysYParser::Rparen, 0);
}

SysYParser::NumberContext* SysYParser::PrimaryExp_Context::number() {
  return getRuleContext<SysYParser::NumberContext>(0);
}

SysYParser::PrimaryExp_Context::PrimaryExp_Context(PrimaryExpContext *ctx) { copyFrom(ctx); }


std::any SysYParser::PrimaryExp_Context::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysYVisitor*>(visitor))
    return parserVisitor->visitPrimaryExp_(this);
  else
    return visitor->visitChildren(this);
}
//----------------- LValExprContext ------------------------------------------------------------------

SysYParser::LValContext* SysYParser::LValExprContext::lVal() {
  return getRuleContext<SysYParser::LValContext>(0);
}

SysYParser::LValExprContext::LValExprContext(PrimaryExpContext *ctx) { copyFrom(ctx); }


std::any SysYParser::LValExprContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysYVisitor*>(visitor))
    return parserVisitor->visitLValExpr(this);
  else
    return visitor->visitChildren(this);
}
SysYParser::PrimaryExpContext* SysYParser::primaryExp() {
  PrimaryExpContext *_localctx = _tracker.createInstance<PrimaryExpContext>(_ctx, getState());
  enterRule(_localctx, 38, SysYParser::RulePrimaryExp);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(259);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case SysYParser::Lparen: {
        _localctx = _tracker.createInstance<SysYParser::PrimaryExp_Context>(_localctx);
        enterOuterAlt(_localctx, 1);
        setState(253);
        match(SysYParser::Lparen);
        setState(254);
        exp();
        setState(255);
        match(SysYParser::Rparen);
        break;
      }

      case SysYParser::Ident: {
        _localctx = _tracker.createInstance<SysYParser::LValExprContext>(_localctx);
        enterOuterAlt(_localctx, 2);
        setState(257);
        lVal();
        break;
      }

      case SysYParser::DecIntConst:
      case SysYParser::OctIntConst:
      case SysYParser::HexIntConst:
      case SysYParser::DecFloatConst:
      case SysYParser::HexFloatConst: {
        _localctx = _tracker.createInstance<SysYParser::PrimaryExp_Context>(_localctx);
        enterOuterAlt(_localctx, 3);
        setState(258);
        number();
        break;
      }

    default:
      throw NoViableAltException(this);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- IntConstContext ------------------------------------------------------------------

SysYParser::IntConstContext::IntConstContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}


size_t SysYParser::IntConstContext::getRuleIndex() const {
  return SysYParser::RuleIntConst;
}

void SysYParser::IntConstContext::copyFrom(IntConstContext *ctx) {
  ParserRuleContext::copyFrom(ctx);
}

//----------------- HexIntConstContext ------------------------------------------------------------------

tree::TerminalNode* SysYParser::HexIntConstContext::HexIntConst() {
  return getToken(SysYParser::HexIntConst, 0);
}

SysYParser::HexIntConstContext::HexIntConstContext(IntConstContext *ctx) { copyFrom(ctx); }


std::any SysYParser::HexIntConstContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysYVisitor*>(visitor))
    return parserVisitor->visitHexIntConst(this);
  else
    return visitor->visitChildren(this);
}
//----------------- DecIntConstContext ------------------------------------------------------------------

tree::TerminalNode* SysYParser::DecIntConstContext::DecIntConst() {
  return getToken(SysYParser::DecIntConst, 0);
}

SysYParser::DecIntConstContext::DecIntConstContext(IntConstContext *ctx) { copyFrom(ctx); }


std::any SysYParser::DecIntConstContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysYVisitor*>(visitor))
    return parserVisitor->visitDecIntConst(this);
  else
    return visitor->visitChildren(this);
}
//----------------- OctIntConstContext ------------------------------------------------------------------

tree::TerminalNode* SysYParser::OctIntConstContext::OctIntConst() {
  return getToken(SysYParser::OctIntConst, 0);
}

SysYParser::OctIntConstContext::OctIntConstContext(IntConstContext *ctx) { copyFrom(ctx); }


std::any SysYParser::OctIntConstContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysYVisitor*>(visitor))
    return parserVisitor->visitOctIntConst(this);
  else
    return visitor->visitChildren(this);
}
SysYParser::IntConstContext* SysYParser::intConst() {
  IntConstContext *_localctx = _tracker.createInstance<IntConstContext>(_ctx, getState());
  enterRule(_localctx, 40, SysYParser::RuleIntConst);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(264);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case SysYParser::DecIntConst: {
        _localctx = _tracker.createInstance<SysYParser::DecIntConstContext>(_localctx);
        enterOuterAlt(_localctx, 1);
        setState(261);
        match(SysYParser::DecIntConst);
        break;
      }

      case SysYParser::OctIntConst: {
        _localctx = _tracker.createInstance<SysYParser::OctIntConstContext>(_localctx);
        enterOuterAlt(_localctx, 2);
        setState(262);
        match(SysYParser::OctIntConst);
        break;
      }

      case SysYParser::HexIntConst: {
        _localctx = _tracker.createInstance<SysYParser::HexIntConstContext>(_localctx);
        enterOuterAlt(_localctx, 3);
        setState(263);
        match(SysYParser::HexIntConst);
        break;
      }

    default:
      throw NoViableAltException(this);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- FloatConstContext ------------------------------------------------------------------

SysYParser::FloatConstContext::FloatConstContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}


size_t SysYParser::FloatConstContext::getRuleIndex() const {
  return SysYParser::RuleFloatConst;
}

void SysYParser::FloatConstContext::copyFrom(FloatConstContext *ctx) {
  ParserRuleContext::copyFrom(ctx);
}

//----------------- DecFloatConstContext ------------------------------------------------------------------

tree::TerminalNode* SysYParser::DecFloatConstContext::DecFloatConst() {
  return getToken(SysYParser::DecFloatConst, 0);
}

SysYParser::DecFloatConstContext::DecFloatConstContext(FloatConstContext *ctx) { copyFrom(ctx); }


std::any SysYParser::DecFloatConstContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysYVisitor*>(visitor))
    return parserVisitor->visitDecFloatConst(this);
  else
    return visitor->visitChildren(this);
}
//----------------- HexFloatConstContext ------------------------------------------------------------------

tree::TerminalNode* SysYParser::HexFloatConstContext::HexFloatConst() {
  return getToken(SysYParser::HexFloatConst, 0);
}

SysYParser::HexFloatConstContext::HexFloatConstContext(FloatConstContext *ctx) { copyFrom(ctx); }


std::any SysYParser::HexFloatConstContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysYVisitor*>(visitor))
    return parserVisitor->visitHexFloatConst(this);
  else
    return visitor->visitChildren(this);
}
SysYParser::FloatConstContext* SysYParser::floatConst() {
  FloatConstContext *_localctx = _tracker.createInstance<FloatConstContext>(_ctx, getState());
  enterRule(_localctx, 42, SysYParser::RuleFloatConst);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(268);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case SysYParser::DecFloatConst: {
        _localctx = _tracker.createInstance<SysYParser::DecFloatConstContext>(_localctx);
        enterOuterAlt(_localctx, 1);
        setState(266);
        match(SysYParser::DecFloatConst);
        break;
      }

      case SysYParser::HexFloatConst: {
        _localctx = _tracker.createInstance<SysYParser::HexFloatConstContext>(_localctx);
        enterOuterAlt(_localctx, 2);
        setState(267);
        match(SysYParser::HexFloatConst);
        break;
      }

    default:
      throw NoViableAltException(this);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- NumberContext ------------------------------------------------------------------

SysYParser::NumberContext::NumberContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

SysYParser::IntConstContext* SysYParser::NumberContext::intConst() {
  return getRuleContext<SysYParser::IntConstContext>(0);
}

SysYParser::FloatConstContext* SysYParser::NumberContext::floatConst() {
  return getRuleContext<SysYParser::FloatConstContext>(0);
}


size_t SysYParser::NumberContext::getRuleIndex() const {
  return SysYParser::RuleNumber;
}


std::any SysYParser::NumberContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysYVisitor*>(visitor))
    return parserVisitor->visitNumber(this);
  else
    return visitor->visitChildren(this);
}

SysYParser::NumberContext* SysYParser::number() {
  NumberContext *_localctx = _tracker.createInstance<NumberContext>(_ctx, getState());
  enterRule(_localctx, 44, SysYParser::RuleNumber);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(272);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case SysYParser::DecIntConst:
      case SysYParser::OctIntConst:
      case SysYParser::HexIntConst: {
        enterOuterAlt(_localctx, 1);
        setState(270);
        intConst();
        break;
      }

      case SysYParser::DecFloatConst:
      case SysYParser::HexFloatConst: {
        enterOuterAlt(_localctx, 2);
        setState(271);
        floatConst();
        break;
      }

    default:
      throw NoViableAltException(this);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- UnaryExpContext ------------------------------------------------------------------

SysYParser::UnaryExpContext::UnaryExpContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}


size_t SysYParser::UnaryExpContext::getRuleIndex() const {
  return SysYParser::RuleUnaryExp;
}

void SysYParser::UnaryExpContext::copyFrom(UnaryExpContext *ctx) {
  ParserRuleContext::copyFrom(ctx);
}

//----------------- CallContext ------------------------------------------------------------------

tree::TerminalNode* SysYParser::CallContext::Ident() {
  return getToken(SysYParser::Ident, 0);
}

tree::TerminalNode* SysYParser::CallContext::Lparen() {
  return getToken(SysYParser::Lparen, 0);
}

tree::TerminalNode* SysYParser::CallContext::Rparen() {
  return getToken(SysYParser::Rparen, 0);
}

SysYParser::FuncRParamsContext* SysYParser::CallContext::funcRParams() {
  return getRuleContext<SysYParser::FuncRParamsContext>(0);
}

SysYParser::CallContext::CallContext(UnaryExpContext *ctx) { copyFrom(ctx); }


std::any SysYParser::CallContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysYVisitor*>(visitor))
    return parserVisitor->visitCall(this);
  else
    return visitor->visitChildren(this);
}
//----------------- NotContext ------------------------------------------------------------------

tree::TerminalNode* SysYParser::NotContext::Not() {
  return getToken(SysYParser::Not, 0);
}

SysYParser::UnaryExpContext* SysYParser::NotContext::unaryExp() {
  return getRuleContext<SysYParser::UnaryExpContext>(0);
}

SysYParser::NotContext::NotContext(UnaryExpContext *ctx) { copyFrom(ctx); }


std::any SysYParser::NotContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysYVisitor*>(visitor))
    return parserVisitor->visitNot(this);
  else
    return visitor->visitChildren(this);
}
//----------------- UnaryExp_Context ------------------------------------------------------------------

SysYParser::PrimaryExpContext* SysYParser::UnaryExp_Context::primaryExp() {
  return getRuleContext<SysYParser::PrimaryExpContext>(0);
}

SysYParser::UnaryExp_Context::UnaryExp_Context(UnaryExpContext *ctx) { copyFrom(ctx); }


std::any SysYParser::UnaryExp_Context::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysYVisitor*>(visitor))
    return parserVisitor->visitUnaryExp_(this);
  else
    return visitor->visitChildren(this);
}
//----------------- UnaryAddContext ------------------------------------------------------------------

tree::TerminalNode* SysYParser::UnaryAddContext::Add() {
  return getToken(SysYParser::Add, 0);
}

SysYParser::UnaryExpContext* SysYParser::UnaryAddContext::unaryExp() {
  return getRuleContext<SysYParser::UnaryExpContext>(0);
}

SysYParser::UnaryAddContext::UnaryAddContext(UnaryExpContext *ctx) { copyFrom(ctx); }


std::any SysYParser::UnaryAddContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysYVisitor*>(visitor))
    return parserVisitor->visitUnaryAdd(this);
  else
    return visitor->visitChildren(this);
}
//----------------- UnarySubContext ------------------------------------------------------------------

tree::TerminalNode* SysYParser::UnarySubContext::Sub() {
  return getToken(SysYParser::Sub, 0);
}

SysYParser::UnaryExpContext* SysYParser::UnarySubContext::unaryExp() {
  return getRuleContext<SysYParser::UnaryExpContext>(0);
}

SysYParser::UnarySubContext::UnarySubContext(UnaryExpContext *ctx) { copyFrom(ctx); }


std::any SysYParser::UnarySubContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysYVisitor*>(visitor))
    return parserVisitor->visitUnarySub(this);
  else
    return visitor->visitChildren(this);
}
SysYParser::UnaryExpContext* SysYParser::unaryExp() {
  UnaryExpContext *_localctx = _tracker.createInstance<UnaryExpContext>(_ctx, getState());
  enterRule(_localctx, 46, SysYParser::RuleUnaryExp);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(287);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 29, _ctx)) {
    case 1: {
      _localctx = _tracker.createInstance<SysYParser::UnaryExp_Context>(_localctx);
      enterOuterAlt(_localctx, 1);
      setState(274);
      primaryExp();
      break;
    }

    case 2: {
      _localctx = _tracker.createInstance<SysYParser::CallContext>(_localctx);
      enterOuterAlt(_localctx, 2);
      setState(275);
      match(SysYParser::Ident);
      setState(276);
      match(SysYParser::Lparen);
      setState(278);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if ((((_la & ~ 0x3fULL) == 0) &&
        ((1ULL << _la) & ((1ULL << SysYParser::Add)
        | (1ULL << SysYParser::Sub)
        | (1ULL << SysYParser::Not)
        | (1ULL << SysYParser::Lparen)
        | (1ULL << SysYParser::Ident)
        | (1ULL << SysYParser::DecIntConst)
        | (1ULL << SysYParser::OctIntConst)
        | (1ULL << SysYParser::HexIntConst)
        | (1ULL << SysYParser::DecFloatConst)
        | (1ULL << SysYParser::HexFloatConst)
        | (1ULL << SysYParser::StringConst))) != 0)) {
        setState(277);
        funcRParams();
      }
      setState(280);
      match(SysYParser::Rparen);
      break;
    }

    case 3: {
      _localctx = _tracker.createInstance<SysYParser::UnaryAddContext>(_localctx);
      enterOuterAlt(_localctx, 3);
      setState(281);
      match(SysYParser::Add);
      setState(282);
      unaryExp();
      break;
    }

    case 4: {
      _localctx = _tracker.createInstance<SysYParser::UnarySubContext>(_localctx);
      enterOuterAlt(_localctx, 4);
      setState(283);
      match(SysYParser::Sub);
      setState(284);
      unaryExp();
      break;
    }

    case 5: {
      _localctx = _tracker.createInstance<SysYParser::NotContext>(_localctx);
      enterOuterAlt(_localctx, 5);
      setState(285);
      match(SysYParser::Not);
      setState(286);
      unaryExp();
      break;
    }

    default:
      break;
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- StringConstContext ------------------------------------------------------------------

SysYParser::StringConstContext::StringConstContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SysYParser::StringConstContext::StringConst() {
  return getToken(SysYParser::StringConst, 0);
}


size_t SysYParser::StringConstContext::getRuleIndex() const {
  return SysYParser::RuleStringConst;
}


std::any SysYParser::StringConstContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysYVisitor*>(visitor))
    return parserVisitor->visitStringConst(this);
  else
    return visitor->visitChildren(this);
}

SysYParser::StringConstContext* SysYParser::stringConst() {
  StringConstContext *_localctx = _tracker.createInstance<StringConstContext>(_ctx, getState());
  enterRule(_localctx, 48, SysYParser::RuleStringConst);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(289);
    match(SysYParser::StringConst);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- FuncRParamContext ------------------------------------------------------------------

SysYParser::FuncRParamContext::FuncRParamContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

SysYParser::ExpContext* SysYParser::FuncRParamContext::exp() {
  return getRuleContext<SysYParser::ExpContext>(0);
}

SysYParser::StringConstContext* SysYParser::FuncRParamContext::stringConst() {
  return getRuleContext<SysYParser::StringConstContext>(0);
}


size_t SysYParser::FuncRParamContext::getRuleIndex() const {
  return SysYParser::RuleFuncRParam;
}


std::any SysYParser::FuncRParamContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysYVisitor*>(visitor))
    return parserVisitor->visitFuncRParam(this);
  else
    return visitor->visitChildren(this);
}

SysYParser::FuncRParamContext* SysYParser::funcRParam() {
  FuncRParamContext *_localctx = _tracker.createInstance<FuncRParamContext>(_ctx, getState());
  enterRule(_localctx, 50, SysYParser::RuleFuncRParam);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(293);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case SysYParser::Add:
      case SysYParser::Sub:
      case SysYParser::Not:
      case SysYParser::Lparen:
      case SysYParser::Ident:
      case SysYParser::DecIntConst:
      case SysYParser::OctIntConst:
      case SysYParser::HexIntConst:
      case SysYParser::DecFloatConst:
      case SysYParser::HexFloatConst: {
        enterOuterAlt(_localctx, 1);
        setState(291);
        exp();
        break;
      }

      case SysYParser::StringConst: {
        enterOuterAlt(_localctx, 2);
        setState(292);
        stringConst();
        break;
      }

    default:
      throw NoViableAltException(this);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- FuncRParamsContext ------------------------------------------------------------------

SysYParser::FuncRParamsContext::FuncRParamsContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<SysYParser::FuncRParamContext *> SysYParser::FuncRParamsContext::funcRParam() {
  return getRuleContexts<SysYParser::FuncRParamContext>();
}

SysYParser::FuncRParamContext* SysYParser::FuncRParamsContext::funcRParam(size_t i) {
  return getRuleContext<SysYParser::FuncRParamContext>(i);
}

std::vector<tree::TerminalNode *> SysYParser::FuncRParamsContext::Comma() {
  return getTokens(SysYParser::Comma);
}

tree::TerminalNode* SysYParser::FuncRParamsContext::Comma(size_t i) {
  return getToken(SysYParser::Comma, i);
}


size_t SysYParser::FuncRParamsContext::getRuleIndex() const {
  return SysYParser::RuleFuncRParams;
}


std::any SysYParser::FuncRParamsContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysYVisitor*>(visitor))
    return parserVisitor->visitFuncRParams(this);
  else
    return visitor->visitChildren(this);
}

SysYParser::FuncRParamsContext* SysYParser::funcRParams() {
  FuncRParamsContext *_localctx = _tracker.createInstance<FuncRParamsContext>(_ctx, getState());
  enterRule(_localctx, 52, SysYParser::RuleFuncRParams);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(295);
    funcRParam();
    setState(300);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == SysYParser::Comma) {
      setState(296);
      match(SysYParser::Comma);
      setState(297);
      funcRParam();
      setState(302);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- MulExpContext ------------------------------------------------------------------

SysYParser::MulExpContext::MulExpContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}


size_t SysYParser::MulExpContext::getRuleIndex() const {
  return SysYParser::RuleMulExp;
}

void SysYParser::MulExpContext::copyFrom(MulExpContext *ctx) {
  ParserRuleContext::copyFrom(ctx);
}

//----------------- DivContext ------------------------------------------------------------------

SysYParser::MulExpContext* SysYParser::DivContext::mulExp() {
  return getRuleContext<SysYParser::MulExpContext>(0);
}

tree::TerminalNode* SysYParser::DivContext::Div() {
  return getToken(SysYParser::Div, 0);
}

SysYParser::UnaryExpContext* SysYParser::DivContext::unaryExp() {
  return getRuleContext<SysYParser::UnaryExpContext>(0);
}

SysYParser::DivContext::DivContext(MulExpContext *ctx) { copyFrom(ctx); }


std::any SysYParser::DivContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysYVisitor*>(visitor))
    return parserVisitor->visitDiv(this);
  else
    return visitor->visitChildren(this);
}
//----------------- ModContext ------------------------------------------------------------------

SysYParser::MulExpContext* SysYParser::ModContext::mulExp() {
  return getRuleContext<SysYParser::MulExpContext>(0);
}

tree::TerminalNode* SysYParser::ModContext::Mod() {
  return getToken(SysYParser::Mod, 0);
}

SysYParser::UnaryExpContext* SysYParser::ModContext::unaryExp() {
  return getRuleContext<SysYParser::UnaryExpContext>(0);
}

SysYParser::ModContext::ModContext(MulExpContext *ctx) { copyFrom(ctx); }


std::any SysYParser::ModContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysYVisitor*>(visitor))
    return parserVisitor->visitMod(this);
  else
    return visitor->visitChildren(this);
}
//----------------- MulContext ------------------------------------------------------------------

SysYParser::MulExpContext* SysYParser::MulContext::mulExp() {
  return getRuleContext<SysYParser::MulExpContext>(0);
}

tree::TerminalNode* SysYParser::MulContext::Mul() {
  return getToken(SysYParser::Mul, 0);
}

SysYParser::UnaryExpContext* SysYParser::MulContext::unaryExp() {
  return getRuleContext<SysYParser::UnaryExpContext>(0);
}

SysYParser::MulContext::MulContext(MulExpContext *ctx) { copyFrom(ctx); }


std::any SysYParser::MulContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysYVisitor*>(visitor))
    return parserVisitor->visitMul(this);
  else
    return visitor->visitChildren(this);
}
//----------------- MulExp_Context ------------------------------------------------------------------

SysYParser::UnaryExpContext* SysYParser::MulExp_Context::unaryExp() {
  return getRuleContext<SysYParser::UnaryExpContext>(0);
}

SysYParser::MulExp_Context::MulExp_Context(MulExpContext *ctx) { copyFrom(ctx); }


std::any SysYParser::MulExp_Context::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysYVisitor*>(visitor))
    return parserVisitor->visitMulExp_(this);
  else
    return visitor->visitChildren(this);
}

SysYParser::MulExpContext* SysYParser::mulExp() {
   return mulExp(0);
}

SysYParser::MulExpContext* SysYParser::mulExp(int precedence) {
  ParserRuleContext *parentContext = _ctx;
  size_t parentState = getState();
  SysYParser::MulExpContext *_localctx = _tracker.createInstance<MulExpContext>(_ctx, parentState);
  SysYParser::MulExpContext *previousContext = _localctx;
  (void)previousContext; // Silence compiler, in case the context is not used by generated code.
  size_t startState = 54;
  enterRecursionRule(_localctx, 54, SysYParser::RuleMulExp, precedence);

    

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    unrollRecursionContexts(parentContext);
  });
  try {
    size_t alt;
    enterOuterAlt(_localctx, 1);
    _localctx = _tracker.createInstance<MulExp_Context>(_localctx);
    _ctx = _localctx;
    previousContext = _localctx;

    setState(304);
    unaryExp();
    _ctx->stop = _input->LT(-1);
    setState(317);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 33, _ctx);
    while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1) {
        if (!_parseListeners.empty())
          triggerExitRuleEvent();
        previousContext = _localctx;
        setState(315);
        _errHandler->sync(this);
        switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 32, _ctx)) {
        case 1: {
          auto newContext = _tracker.createInstance<MulContext>(_tracker.createInstance<MulExpContext>(parentContext, parentState));
          _localctx = newContext;
          pushNewRecursionContext(newContext, startState, RuleMulExp);
          setState(306);

          if (!(precpred(_ctx, 3))) throw FailedPredicateException(this, "precpred(_ctx, 3)");
          setState(307);
          match(SysYParser::Mul);
          setState(308);
          unaryExp();
          break;
        }

        case 2: {
          auto newContext = _tracker.createInstance<DivContext>(_tracker.createInstance<MulExpContext>(parentContext, parentState));
          _localctx = newContext;
          pushNewRecursionContext(newContext, startState, RuleMulExp);
          setState(309);

          if (!(precpred(_ctx, 2))) throw FailedPredicateException(this, "precpred(_ctx, 2)");
          setState(310);
          match(SysYParser::Div);
          setState(311);
          unaryExp();
          break;
        }

        case 3: {
          auto newContext = _tracker.createInstance<ModContext>(_tracker.createInstance<MulExpContext>(parentContext, parentState));
          _localctx = newContext;
          pushNewRecursionContext(newContext, startState, RuleMulExp);
          setState(312);

          if (!(precpred(_ctx, 1))) throw FailedPredicateException(this, "precpred(_ctx, 1)");
          setState(313);
          match(SysYParser::Mod);
          setState(314);
          unaryExp();
          break;
        }

        default:
          break;
        } 
      }
      setState(319);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 33, _ctx);
    }
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }
  return _localctx;
}

//----------------- AddExpContext ------------------------------------------------------------------

SysYParser::AddExpContext::AddExpContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}


size_t SysYParser::AddExpContext::getRuleIndex() const {
  return SysYParser::RuleAddExp;
}

void SysYParser::AddExpContext::copyFrom(AddExpContext *ctx) {
  ParserRuleContext::copyFrom(ctx);
}

//----------------- AddExp_Context ------------------------------------------------------------------

SysYParser::MulExpContext* SysYParser::AddExp_Context::mulExp() {
  return getRuleContext<SysYParser::MulExpContext>(0);
}

SysYParser::AddExp_Context::AddExp_Context(AddExpContext *ctx) { copyFrom(ctx); }


std::any SysYParser::AddExp_Context::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysYVisitor*>(visitor))
    return parserVisitor->visitAddExp_(this);
  else
    return visitor->visitChildren(this);
}
//----------------- AddContext ------------------------------------------------------------------

SysYParser::AddExpContext* SysYParser::AddContext::addExp() {
  return getRuleContext<SysYParser::AddExpContext>(0);
}

tree::TerminalNode* SysYParser::AddContext::Add() {
  return getToken(SysYParser::Add, 0);
}

SysYParser::MulExpContext* SysYParser::AddContext::mulExp() {
  return getRuleContext<SysYParser::MulExpContext>(0);
}

SysYParser::AddContext::AddContext(AddExpContext *ctx) { copyFrom(ctx); }


std::any SysYParser::AddContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysYVisitor*>(visitor))
    return parserVisitor->visitAdd(this);
  else
    return visitor->visitChildren(this);
}
//----------------- SubContext ------------------------------------------------------------------

SysYParser::AddExpContext* SysYParser::SubContext::addExp() {
  return getRuleContext<SysYParser::AddExpContext>(0);
}

tree::TerminalNode* SysYParser::SubContext::Sub() {
  return getToken(SysYParser::Sub, 0);
}

SysYParser::MulExpContext* SysYParser::SubContext::mulExp() {
  return getRuleContext<SysYParser::MulExpContext>(0);
}

SysYParser::SubContext::SubContext(AddExpContext *ctx) { copyFrom(ctx); }


std::any SysYParser::SubContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysYVisitor*>(visitor))
    return parserVisitor->visitSub(this);
  else
    return visitor->visitChildren(this);
}

SysYParser::AddExpContext* SysYParser::addExp() {
   return addExp(0);
}

SysYParser::AddExpContext* SysYParser::addExp(int precedence) {
  ParserRuleContext *parentContext = _ctx;
  size_t parentState = getState();
  SysYParser::AddExpContext *_localctx = _tracker.createInstance<AddExpContext>(_ctx, parentState);
  SysYParser::AddExpContext *previousContext = _localctx;
  (void)previousContext; // Silence compiler, in case the context is not used by generated code.
  size_t startState = 56;
  enterRecursionRule(_localctx, 56, SysYParser::RuleAddExp, precedence);

    

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    unrollRecursionContexts(parentContext);
  });
  try {
    size_t alt;
    enterOuterAlt(_localctx, 1);
    _localctx = _tracker.createInstance<AddExp_Context>(_localctx);
    _ctx = _localctx;
    previousContext = _localctx;

    setState(321);
    mulExp(0);
    _ctx->stop = _input->LT(-1);
    setState(331);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 35, _ctx);
    while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1) {
        if (!_parseListeners.empty())
          triggerExitRuleEvent();
        previousContext = _localctx;
        setState(329);
        _errHandler->sync(this);
        switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 34, _ctx)) {
        case 1: {
          auto newContext = _tracker.createInstance<AddContext>(_tracker.createInstance<AddExpContext>(parentContext, parentState));
          _localctx = newContext;
          pushNewRecursionContext(newContext, startState, RuleAddExp);
          setState(323);

          if (!(precpred(_ctx, 2))) throw FailedPredicateException(this, "precpred(_ctx, 2)");
          setState(324);
          match(SysYParser::Add);
          setState(325);
          mulExp(0);
          break;
        }

        case 2: {
          auto newContext = _tracker.createInstance<SubContext>(_tracker.createInstance<AddExpContext>(parentContext, parentState));
          _localctx = newContext;
          pushNewRecursionContext(newContext, startState, RuleAddExp);
          setState(326);

          if (!(precpred(_ctx, 1))) throw FailedPredicateException(this, "precpred(_ctx, 1)");
          setState(327);
          match(SysYParser::Sub);
          setState(328);
          mulExp(0);
          break;
        }

        default:
          break;
        } 
      }
      setState(333);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 35, _ctx);
    }
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }
  return _localctx;
}

//----------------- RelExpContext ------------------------------------------------------------------

SysYParser::RelExpContext::RelExpContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}


size_t SysYParser::RelExpContext::getRuleIndex() const {
  return SysYParser::RuleRelExp;
}

void SysYParser::RelExpContext::copyFrom(RelExpContext *ctx) {
  ParserRuleContext::copyFrom(ctx);
}

//----------------- GeqContext ------------------------------------------------------------------

SysYParser::RelExpContext* SysYParser::GeqContext::relExp() {
  return getRuleContext<SysYParser::RelExpContext>(0);
}

tree::TerminalNode* SysYParser::GeqContext::Geq() {
  return getToken(SysYParser::Geq, 0);
}

SysYParser::AddExpContext* SysYParser::GeqContext::addExp() {
  return getRuleContext<SysYParser::AddExpContext>(0);
}

SysYParser::GeqContext::GeqContext(RelExpContext *ctx) { copyFrom(ctx); }


std::any SysYParser::GeqContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysYVisitor*>(visitor))
    return parserVisitor->visitGeq(this);
  else
    return visitor->visitChildren(this);
}
//----------------- LtContext ------------------------------------------------------------------

SysYParser::RelExpContext* SysYParser::LtContext::relExp() {
  return getRuleContext<SysYParser::RelExpContext>(0);
}

tree::TerminalNode* SysYParser::LtContext::Lt() {
  return getToken(SysYParser::Lt, 0);
}

SysYParser::AddExpContext* SysYParser::LtContext::addExp() {
  return getRuleContext<SysYParser::AddExpContext>(0);
}

SysYParser::LtContext::LtContext(RelExpContext *ctx) { copyFrom(ctx); }


std::any SysYParser::LtContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysYVisitor*>(visitor))
    return parserVisitor->visitLt(this);
  else
    return visitor->visitChildren(this);
}
//----------------- RelExp_Context ------------------------------------------------------------------

SysYParser::AddExpContext* SysYParser::RelExp_Context::addExp() {
  return getRuleContext<SysYParser::AddExpContext>(0);
}

SysYParser::RelExp_Context::RelExp_Context(RelExpContext *ctx) { copyFrom(ctx); }


std::any SysYParser::RelExp_Context::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysYVisitor*>(visitor))
    return parserVisitor->visitRelExp_(this);
  else
    return visitor->visitChildren(this);
}
//----------------- LeqContext ------------------------------------------------------------------

SysYParser::RelExpContext* SysYParser::LeqContext::relExp() {
  return getRuleContext<SysYParser::RelExpContext>(0);
}

tree::TerminalNode* SysYParser::LeqContext::Leq() {
  return getToken(SysYParser::Leq, 0);
}

SysYParser::AddExpContext* SysYParser::LeqContext::addExp() {
  return getRuleContext<SysYParser::AddExpContext>(0);
}

SysYParser::LeqContext::LeqContext(RelExpContext *ctx) { copyFrom(ctx); }


std::any SysYParser::LeqContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysYVisitor*>(visitor))
    return parserVisitor->visitLeq(this);
  else
    return visitor->visitChildren(this);
}
//----------------- GtContext ------------------------------------------------------------------

SysYParser::RelExpContext* SysYParser::GtContext::relExp() {
  return getRuleContext<SysYParser::RelExpContext>(0);
}

tree::TerminalNode* SysYParser::GtContext::Gt() {
  return getToken(SysYParser::Gt, 0);
}

SysYParser::AddExpContext* SysYParser::GtContext::addExp() {
  return getRuleContext<SysYParser::AddExpContext>(0);
}

SysYParser::GtContext::GtContext(RelExpContext *ctx) { copyFrom(ctx); }


std::any SysYParser::GtContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysYVisitor*>(visitor))
    return parserVisitor->visitGt(this);
  else
    return visitor->visitChildren(this);
}

SysYParser::RelExpContext* SysYParser::relExp() {
   return relExp(0);
}

SysYParser::RelExpContext* SysYParser::relExp(int precedence) {
  ParserRuleContext *parentContext = _ctx;
  size_t parentState = getState();
  SysYParser::RelExpContext *_localctx = _tracker.createInstance<RelExpContext>(_ctx, parentState);
  SysYParser::RelExpContext *previousContext = _localctx;
  (void)previousContext; // Silence compiler, in case the context is not used by generated code.
  size_t startState = 58;
  enterRecursionRule(_localctx, 58, SysYParser::RuleRelExp, precedence);

    

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    unrollRecursionContexts(parentContext);
  });
  try {
    size_t alt;
    enterOuterAlt(_localctx, 1);
    _localctx = _tracker.createInstance<RelExp_Context>(_localctx);
    _ctx = _localctx;
    previousContext = _localctx;

    setState(335);
    addExp(0);
    _ctx->stop = _input->LT(-1);
    setState(351);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 37, _ctx);
    while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1) {
        if (!_parseListeners.empty())
          triggerExitRuleEvent();
        previousContext = _localctx;
        setState(349);
        _errHandler->sync(this);
        switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 36, _ctx)) {
        case 1: {
          auto newContext = _tracker.createInstance<LtContext>(_tracker.createInstance<RelExpContext>(parentContext, parentState));
          _localctx = newContext;
          pushNewRecursionContext(newContext, startState, RuleRelExp);
          setState(337);

          if (!(precpred(_ctx, 4))) throw FailedPredicateException(this, "precpred(_ctx, 4)");
          setState(338);
          match(SysYParser::Lt);
          setState(339);
          addExp(0);
          break;
        }

        case 2: {
          auto newContext = _tracker.createInstance<GtContext>(_tracker.createInstance<RelExpContext>(parentContext, parentState));
          _localctx = newContext;
          pushNewRecursionContext(newContext, startState, RuleRelExp);
          setState(340);

          if (!(precpred(_ctx, 3))) throw FailedPredicateException(this, "precpred(_ctx, 3)");
          setState(341);
          match(SysYParser::Gt);
          setState(342);
          addExp(0);
          break;
        }

        case 3: {
          auto newContext = _tracker.createInstance<LeqContext>(_tracker.createInstance<RelExpContext>(parentContext, parentState));
          _localctx = newContext;
          pushNewRecursionContext(newContext, startState, RuleRelExp);
          setState(343);

          if (!(precpred(_ctx, 2))) throw FailedPredicateException(this, "precpred(_ctx, 2)");
          setState(344);
          match(SysYParser::Leq);
          setState(345);
          addExp(0);
          break;
        }

        case 4: {
          auto newContext = _tracker.createInstance<GeqContext>(_tracker.createInstance<RelExpContext>(parentContext, parentState));
          _localctx = newContext;
          pushNewRecursionContext(newContext, startState, RuleRelExp);
          setState(346);

          if (!(precpred(_ctx, 1))) throw FailedPredicateException(this, "precpred(_ctx, 1)");
          setState(347);
          match(SysYParser::Geq);
          setState(348);
          addExp(0);
          break;
        }

        default:
          break;
        } 
      }
      setState(353);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 37, _ctx);
    }
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }
  return _localctx;
}

//----------------- EqExpContext ------------------------------------------------------------------

SysYParser::EqExpContext::EqExpContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}


size_t SysYParser::EqExpContext::getRuleIndex() const {
  return SysYParser::RuleEqExp;
}

void SysYParser::EqExpContext::copyFrom(EqExpContext *ctx) {
  ParserRuleContext::copyFrom(ctx);
}

//----------------- NeqContext ------------------------------------------------------------------

SysYParser::EqExpContext* SysYParser::NeqContext::eqExp() {
  return getRuleContext<SysYParser::EqExpContext>(0);
}

tree::TerminalNode* SysYParser::NeqContext::Neq() {
  return getToken(SysYParser::Neq, 0);
}

SysYParser::RelExpContext* SysYParser::NeqContext::relExp() {
  return getRuleContext<SysYParser::RelExpContext>(0);
}

SysYParser::NeqContext::NeqContext(EqExpContext *ctx) { copyFrom(ctx); }


std::any SysYParser::NeqContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysYVisitor*>(visitor))
    return parserVisitor->visitNeq(this);
  else
    return visitor->visitChildren(this);
}
//----------------- EqContext ------------------------------------------------------------------

SysYParser::EqExpContext* SysYParser::EqContext::eqExp() {
  return getRuleContext<SysYParser::EqExpContext>(0);
}

tree::TerminalNode* SysYParser::EqContext::Eq() {
  return getToken(SysYParser::Eq, 0);
}

SysYParser::RelExpContext* SysYParser::EqContext::relExp() {
  return getRuleContext<SysYParser::RelExpContext>(0);
}

SysYParser::EqContext::EqContext(EqExpContext *ctx) { copyFrom(ctx); }


std::any SysYParser::EqContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysYVisitor*>(visitor))
    return parserVisitor->visitEq(this);
  else
    return visitor->visitChildren(this);
}
//----------------- EqExp_Context ------------------------------------------------------------------

SysYParser::RelExpContext* SysYParser::EqExp_Context::relExp() {
  return getRuleContext<SysYParser::RelExpContext>(0);
}

SysYParser::EqExp_Context::EqExp_Context(EqExpContext *ctx) { copyFrom(ctx); }


std::any SysYParser::EqExp_Context::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysYVisitor*>(visitor))
    return parserVisitor->visitEqExp_(this);
  else
    return visitor->visitChildren(this);
}

SysYParser::EqExpContext* SysYParser::eqExp() {
   return eqExp(0);
}

SysYParser::EqExpContext* SysYParser::eqExp(int precedence) {
  ParserRuleContext *parentContext = _ctx;
  size_t parentState = getState();
  SysYParser::EqExpContext *_localctx = _tracker.createInstance<EqExpContext>(_ctx, parentState);
  SysYParser::EqExpContext *previousContext = _localctx;
  (void)previousContext; // Silence compiler, in case the context is not used by generated code.
  size_t startState = 60;
  enterRecursionRule(_localctx, 60, SysYParser::RuleEqExp, precedence);

    

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    unrollRecursionContexts(parentContext);
  });
  try {
    size_t alt;
    enterOuterAlt(_localctx, 1);
    _localctx = _tracker.createInstance<EqExp_Context>(_localctx);
    _ctx = _localctx;
    previousContext = _localctx;

    setState(355);
    relExp(0);
    _ctx->stop = _input->LT(-1);
    setState(365);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 39, _ctx);
    while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1) {
        if (!_parseListeners.empty())
          triggerExitRuleEvent();
        previousContext = _localctx;
        setState(363);
        _errHandler->sync(this);
        switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 38, _ctx)) {
        case 1: {
          auto newContext = _tracker.createInstance<EqContext>(_tracker.createInstance<EqExpContext>(parentContext, parentState));
          _localctx = newContext;
          pushNewRecursionContext(newContext, startState, RuleEqExp);
          setState(357);

          if (!(precpred(_ctx, 2))) throw FailedPredicateException(this, "precpred(_ctx, 2)");
          setState(358);
          match(SysYParser::Eq);
          setState(359);
          relExp(0);
          break;
        }

        case 2: {
          auto newContext = _tracker.createInstance<NeqContext>(_tracker.createInstance<EqExpContext>(parentContext, parentState));
          _localctx = newContext;
          pushNewRecursionContext(newContext, startState, RuleEqExp);
          setState(360);

          if (!(precpred(_ctx, 1))) throw FailedPredicateException(this, "precpred(_ctx, 1)");
          setState(361);
          match(SysYParser::Neq);
          setState(362);
          relExp(0);
          break;
        }

        default:
          break;
        } 
      }
      setState(367);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 39, _ctx);
    }
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }
  return _localctx;
}

//----------------- LAndExpContext ------------------------------------------------------------------

SysYParser::LAndExpContext::LAndExpContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}


size_t SysYParser::LAndExpContext::getRuleIndex() const {
  return SysYParser::RuleLAndExp;
}

void SysYParser::LAndExpContext::copyFrom(LAndExpContext *ctx) {
  ParserRuleContext::copyFrom(ctx);
}

//----------------- LAndExp_Context ------------------------------------------------------------------

SysYParser::EqExpContext* SysYParser::LAndExp_Context::eqExp() {
  return getRuleContext<SysYParser::EqExpContext>(0);
}

SysYParser::LAndExp_Context::LAndExp_Context(LAndExpContext *ctx) { copyFrom(ctx); }


std::any SysYParser::LAndExp_Context::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysYVisitor*>(visitor))
    return parserVisitor->visitLAndExp_(this);
  else
    return visitor->visitChildren(this);
}
//----------------- AndContext ------------------------------------------------------------------

SysYParser::LAndExpContext* SysYParser::AndContext::lAndExp() {
  return getRuleContext<SysYParser::LAndExpContext>(0);
}

tree::TerminalNode* SysYParser::AndContext::And() {
  return getToken(SysYParser::And, 0);
}

SysYParser::EqExpContext* SysYParser::AndContext::eqExp() {
  return getRuleContext<SysYParser::EqExpContext>(0);
}

SysYParser::AndContext::AndContext(LAndExpContext *ctx) { copyFrom(ctx); }


std::any SysYParser::AndContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysYVisitor*>(visitor))
    return parserVisitor->visitAnd(this);
  else
    return visitor->visitChildren(this);
}

SysYParser::LAndExpContext* SysYParser::lAndExp() {
   return lAndExp(0);
}

SysYParser::LAndExpContext* SysYParser::lAndExp(int precedence) {
  ParserRuleContext *parentContext = _ctx;
  size_t parentState = getState();
  SysYParser::LAndExpContext *_localctx = _tracker.createInstance<LAndExpContext>(_ctx, parentState);
  SysYParser::LAndExpContext *previousContext = _localctx;
  (void)previousContext; // Silence compiler, in case the context is not used by generated code.
  size_t startState = 62;
  enterRecursionRule(_localctx, 62, SysYParser::RuleLAndExp, precedence);

    

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    unrollRecursionContexts(parentContext);
  });
  try {
    size_t alt;
    enterOuterAlt(_localctx, 1);
    _localctx = _tracker.createInstance<LAndExp_Context>(_localctx);
    _ctx = _localctx;
    previousContext = _localctx;

    setState(369);
    eqExp(0);
    _ctx->stop = _input->LT(-1);
    setState(376);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 40, _ctx);
    while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1) {
        if (!_parseListeners.empty())
          triggerExitRuleEvent();
        previousContext = _localctx;
        auto newContext = _tracker.createInstance<AndContext>(_tracker.createInstance<LAndExpContext>(parentContext, parentState));
        _localctx = newContext;
        pushNewRecursionContext(newContext, startState, RuleLAndExp);
        setState(371);

        if (!(precpred(_ctx, 1))) throw FailedPredicateException(this, "precpred(_ctx, 1)");
        setState(372);
        match(SysYParser::And);
        setState(373);
        eqExp(0); 
      }
      setState(378);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 40, _ctx);
    }
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }
  return _localctx;
}

//----------------- LOrExpContext ------------------------------------------------------------------

SysYParser::LOrExpContext::LOrExpContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}


size_t SysYParser::LOrExpContext::getRuleIndex() const {
  return SysYParser::RuleLOrExp;
}

void SysYParser::LOrExpContext::copyFrom(LOrExpContext *ctx) {
  ParserRuleContext::copyFrom(ctx);
}

//----------------- OrContext ------------------------------------------------------------------

SysYParser::LOrExpContext* SysYParser::OrContext::lOrExp() {
  return getRuleContext<SysYParser::LOrExpContext>(0);
}

tree::TerminalNode* SysYParser::OrContext::Or() {
  return getToken(SysYParser::Or, 0);
}

SysYParser::LAndExpContext* SysYParser::OrContext::lAndExp() {
  return getRuleContext<SysYParser::LAndExpContext>(0);
}

SysYParser::OrContext::OrContext(LOrExpContext *ctx) { copyFrom(ctx); }


std::any SysYParser::OrContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysYVisitor*>(visitor))
    return parserVisitor->visitOr(this);
  else
    return visitor->visitChildren(this);
}
//----------------- LOrExp_Context ------------------------------------------------------------------

SysYParser::LAndExpContext* SysYParser::LOrExp_Context::lAndExp() {
  return getRuleContext<SysYParser::LAndExpContext>(0);
}

SysYParser::LOrExp_Context::LOrExp_Context(LOrExpContext *ctx) { copyFrom(ctx); }


std::any SysYParser::LOrExp_Context::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysYVisitor*>(visitor))
    return parserVisitor->visitLOrExp_(this);
  else
    return visitor->visitChildren(this);
}

SysYParser::LOrExpContext* SysYParser::lOrExp() {
   return lOrExp(0);
}

SysYParser::LOrExpContext* SysYParser::lOrExp(int precedence) {
  ParserRuleContext *parentContext = _ctx;
  size_t parentState = getState();
  SysYParser::LOrExpContext *_localctx = _tracker.createInstance<LOrExpContext>(_ctx, parentState);
  SysYParser::LOrExpContext *previousContext = _localctx;
  (void)previousContext; // Silence compiler, in case the context is not used by generated code.
  size_t startState = 64;
  enterRecursionRule(_localctx, 64, SysYParser::RuleLOrExp, precedence);

    

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    unrollRecursionContexts(parentContext);
  });
  try {
    size_t alt;
    enterOuterAlt(_localctx, 1);
    _localctx = _tracker.createInstance<LOrExp_Context>(_localctx);
    _ctx = _localctx;
    previousContext = _localctx;

    setState(380);
    lAndExp(0);
    _ctx->stop = _input->LT(-1);
    setState(387);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 41, _ctx);
    while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1) {
        if (!_parseListeners.empty())
          triggerExitRuleEvent();
        previousContext = _localctx;
        auto newContext = _tracker.createInstance<OrContext>(_tracker.createInstance<LOrExpContext>(parentContext, parentState));
        _localctx = newContext;
        pushNewRecursionContext(newContext, startState, RuleLOrExp);
        setState(382);

        if (!(precpred(_ctx, 1))) throw FailedPredicateException(this, "precpred(_ctx, 1)");
        setState(383);
        match(SysYParser::Or);
        setState(384);
        lAndExp(0); 
      }
      setState(389);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 41, _ctx);
    }
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }
  return _localctx;
}

bool SysYParser::sempred(RuleContext *context, size_t ruleIndex, size_t predicateIndex) {
  switch (ruleIndex) {
    case 27: return mulExpSempred(antlrcpp::downCast<MulExpContext *>(context), predicateIndex);
    case 28: return addExpSempred(antlrcpp::downCast<AddExpContext *>(context), predicateIndex);
    case 29: return relExpSempred(antlrcpp::downCast<RelExpContext *>(context), predicateIndex);
    case 30: return eqExpSempred(antlrcpp::downCast<EqExpContext *>(context), predicateIndex);
    case 31: return lAndExpSempred(antlrcpp::downCast<LAndExpContext *>(context), predicateIndex);
    case 32: return lOrExpSempred(antlrcpp::downCast<LOrExpContext *>(context), predicateIndex);

  default:
    break;
  }
  return true;
}

bool SysYParser::mulExpSempred(MulExpContext *_localctx, size_t predicateIndex) {
  switch (predicateIndex) {
    case 0: return precpred(_ctx, 3);
    case 1: return precpred(_ctx, 2);
    case 2: return precpred(_ctx, 1);

  default:
    break;
  }
  return true;
}

bool SysYParser::addExpSempred(AddExpContext *_localctx, size_t predicateIndex) {
  switch (predicateIndex) {
    case 3: return precpred(_ctx, 2);
    case 4: return precpred(_ctx, 1);

  default:
    break;
  }
  return true;
}

bool SysYParser::relExpSempred(RelExpContext *_localctx, size_t predicateIndex) {
  switch (predicateIndex) {
    case 5: return precpred(_ctx, 4);
    case 6: return precpred(_ctx, 3);
    case 7: return precpred(_ctx, 2);
    case 8: return precpred(_ctx, 1);

  default:
    break;
  }
  return true;
}

bool SysYParser::eqExpSempred(EqExpContext *_localctx, size_t predicateIndex) {
  switch (predicateIndex) {
    case 9: return precpred(_ctx, 2);
    case 10: return precpred(_ctx, 1);

  default:
    break;
  }
  return true;
}

bool SysYParser::lAndExpSempred(LAndExpContext *_localctx, size_t predicateIndex) {
  switch (predicateIndex) {
    case 11: return precpred(_ctx, 1);

  default:
    break;
  }
  return true;
}

bool SysYParser::lOrExpSempred(LOrExpContext *_localctx, size_t predicateIndex) {
  switch (predicateIndex) {
    case 12: return precpred(_ctx, 1);

  default:
    break;
  }
  return true;
}

void SysYParser::initialize() {
  std::call_once(sysyParserOnceFlag, sysyParserInitialize);
}
