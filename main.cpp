#include "linenoise.hpp"
#include "peglib.h"
using namespace std;

typedef shared_ptr<peg::Ast> AST;

const auto grammar = R"(
  NUMBER <- '-'? [0-9]+ ('.' [0-9]+)?
)";

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
      // ASTの表示
      cout << peg::ast_to_s(ast);

      // 入力された行を履歴に追加
      linenoise::AddHistory(line.c_str());
    } else {
      cout << "syntax error..." << endl;
    }
  }
}