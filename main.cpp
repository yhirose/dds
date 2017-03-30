#include "linenoise.hpp"
#include "peglib.h"
using namespace std;

typedef shared_ptr<peg::Ast> AST;

// 文法
const auto grammar = R"(
  # ルール
  START              <- _ EXPRESSION
  EXPRESSION         <- ASSIGNMENT / ADDITIVE
  ASSIGNMENT         <- IDENTIFIER '=' _ EXPRESSION
  ADDITIVE           <- MULTITIVE (ADDITIVE_OPERATOR MULTITIVE)*
  MULTITIVE          <- PRIMARY (MULTITIVE_OPERATOR PRIMARY)*
  PRIMARY            <- NUMBER / VARIABLE / '(' _ EXPRESSION ')' _
  VARIABLE           <- IDENTIFIER

  # トークン
  ADDITIVE_OPERATOR  <- < [-+] > _
  MULTITIVE_OPERATOR <- < [/*] > _
  NUMBER             <- < '-'? [0-9]+ ('.' [0-9]+)? > _
  IDENTIFIER         <- < [a-zA-Z][a-zA-Z0-9]* > _

  # 空白文字
  ~_                 <- [ \n\r\t]*
)";

// 変数表
map<string, double> variables;

// 評価器
double eval(const AST& ast) {
  if (ast->name == "ADDITIVE" ||
      ast->name == "MULTITIVE") {
    // 二項演算子の計算
    auto result = eval(ast->nodes[0]);
    for (auto i = 1u; i < ast->nodes.size(); i += 2) {
      auto num = eval(ast->nodes[i + 1]);
      auto ope = ast->nodes[i]->token[0];
      switch (ope) {
        case '+': result += num; break;
        case '-': result -= num; break;
        case '*': result *= num; break;
        case '/':
          // ゼロ除算チェック
          if (num == 0) {
            throw std::runtime_error("divide by 0 error");
          }
          result /= num;
          break;
      }
    }
    return result;
  } else if (ast->name == "ASSIGNMENT") {
    // 変数名の取得
    auto sym = ast->nodes[0]->token;

    // 値の取得
    auto val = eval(ast->nodes[1]);

    // 変数表に設定
    variables[sym] = val;

    return val;
  } else if (ast->name == "VARIABLE") {
    // 変数名の取得
    auto sym = ast->nodes[0]->token;

    // 変数が存在するかチェック
    auto it = variables.find(sym);
    if (it == variables.end()) {
      throw std::runtime_error("undefined variable: '" + sym + "'");
    }
    return it->second;
  } else if (ast->name == "NUMBER") {
    // 数字文字列をdoubleに変換
    return stod(ast->token);
  }

  // NOTREACHED
  assert(false);
  return 0;
}

int main() {
  // パーサーの生成
  peg::parser parser(grammar);
  assert(parser);

  // 抽象構文木(AST)生成機能を使用する
  parser.enable_ast();

  // REPL
  while (true) {
    // 行の入力
    string line;
    auto quit = linenoise::Readline("> ", line);

    // CTRL+C が押されたらループを抜ける
    if (quit) {
      break;
    }

    // 入力行をパースし、ASTを生成
    AST ast;
    auto ret = parser.parse(line.c_str(), ast);

    if (ret) {
      // 冗長なASTノードを除去 ('VARIABLE'ルールを除く)
      ast = peg::optimize_ast(ast, { "VARIABLE" });

      // ASTの表示
      cout << peg::ast_to_s(ast);

      // ASTの評価
      try {
        auto result = eval(ast);
        cout << result << endl;

        // 入力された行を履歴に追加
        linenoise::AddHistory(line.c_str());
      } catch (const std::runtime_error& e) {
        cout << e.what() << endl;
      }
    } else {
      cout << "syntax error..." << endl;
    }
  }
}