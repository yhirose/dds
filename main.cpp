#include "linenoise.hpp"
using namespace std;

int main() {
  // REPL
  while (true) {
    // 行の入力
    string line;
    auto quit = linenoise::Readline("> ", line);

    // CTRL+C が押されたらループを抜ける
    if (quit) {
      break;
    }

    // 入力された文字列の表示
    cout << line << endl;

    // 入力された行を履歴に追加
    linenoise::AddHistory(line.c_str());
  }
}