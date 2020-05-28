#include <curses.h>
#include <stdlib.h>
#include <time.h>

//ぷよの色を表す列挙型
enum puyocolor { NONE, RED, BLUE, GREEN, YELLOW, WALL };

struct puyodata {
  enum puyocolor color;
  bool handling;

  puyodata() {
    color = NONE;
    handling = false;
  }
};

class Field {
 private:
  struct puyodata *field_array;
  unsigned int field_line, field_column;

  void Release() {
    if (field_array == NULL) {
      return;
    }
    delete[] field_array;
    field_array = NULL;
  }

 public:
  Field() : field_array(NULL), field_line(0), field_column(0) {}
  ~Field() { Release(); }

  void ChangeSize(unsigned int line, unsigned int column) {
    Release();

    field_array = new struct puyodata[line * column];

    field_line = line;
    field_column = column;
  }

  struct puyodata GetValue(unsigned int y, unsigned int x) {
    if (y >= field_line || x >= field_column) {
      struct puyodata temp;
      return temp;
    }
    return field_array[y * field_column + x];
  }

  void SetValue(unsigned int y, unsigned int x, struct puyodata data) {
    if (y >= field_line || x >= field_column) {
      return;
    }
    field_array[y * field_column + x] = data;
  }

  unsigned int GetLine() { return field_line; }

  unsigned int GetColumn() { return field_column; }
};

class FieldControl : public Field {
 private:
  int puyorotate, x1, y1, x2, y2;
  struct puyodata puyo1, puyo2, nextpuyo1, nextpuyo2, nextnextpuyo1,
      nextnextpuyo2;

 public:
  //次と次の次に表示されるぷよの色を格納　{nextpuyo1, nextpuyo2, nextnextpuyo1,
  // nextnextpuyo2}の順番で格納
  puyocolor next_puyo_color[4];

  FieldControl() {
    ChangeSize(15, 8);
    struct puyodata wall;
    wall.color = WALL;

    for (int y = 0; y < GetLine(); y++) {
      for (int x = 0; x < GetColumn(); x++) {
        if (x == 0 || x == GetColumn() - 1 || y == 0 || y == GetLine() - 1) {
          SetValue(y, x, wall);
        }
      }
    }

    puyocolor color_array[] = {RED, BLUE, GREEN, YELLOW};

    nextpuyo1.color = color_array[rand() % 4];
    nextpuyo1.handling = true;

    nextpuyo2.color = color_array[rand() % 4];
    nextpuyo2.handling = true;

    nextnextpuyo1.color = color_array[rand() % 4];
    nextnextpuyo1.handling = true;

    nextnextpuyo2.color = color_array[rand() % 4];
    nextnextpuyo2.handling = true;
  }

  bool GeneratePuyo() {
    if (GetValue(2, 3).color != NONE) {
      return false;
    }

    puyocolor color_array[] = {RED, BLUE, GREEN, YELLOW};

    puyorotate = 0;

    puyo1 = nextpuyo1;
    puyo2 = nextpuyo2;

    nextpuyo1 = nextnextpuyo1;
    nextpuyo2 = nextnextpuyo2;

    nextnextpuyo1.color = color_array[rand() % 4];
    nextnextpuyo1.handling = true;

    nextnextpuyo2.color = color_array[rand() % 4];
    nextnextpuyo2.handling = true;

    x1 = 3;
    y1 = 2;
    x2 = 3;
    y2 = 1;

    SetValue(y1, x1, puyo1);
    SetValue(y2, x2, puyo2);

    next_puyo_color[0] = nextpuyo1.color;
    next_puyo_color[1] = nextpuyo2.color;
    next_puyo_color[2] = nextnextpuyo1.color;
    next_puyo_color[3] = nextnextpuyo2.color;

    return true;
  }

  bool ControlDown() {
    struct puyodata init_data;

    if ((GetValue(y1 + 1, x1).color != NONE &&
         GetValue(y1 + 1, x1).handling == false) ||
        (GetValue(y2 + 1, x2).color != NONE &&
         GetValue(y2 + 1, x2).handling == false)) {
      SetValue(y1, x1, puyo1);
      SetValue(y2, x2, puyo2);
      return true;
    }

    SetValue(y1, x1, init_data);
    SetValue(y2, x2, init_data);
    SetValue(++y1, x1, puyo1);
    SetValue(++y2, x2, puyo2);

    return false;
  }

  void ControlLeft() {
    struct puyodata init_data;
    if (!((GetValue(y1, x1 - 1).color != NONE &&
           GetValue(y1, x1 - 1).handling == false) ||
          (GetValue(y2, x2 - 1).color != NONE &&
           GetValue(y2, x2 - 1).handling == false))) {
      SetValue(y1, x1, init_data);
      SetValue(y2, x2, init_data);
      SetValue(y1, --x1, puyo1);
      SetValue(y2, --x2, puyo2);
    }
  }

  void ControlRight() {
    struct puyodata init_data;
    if (!((GetValue(y1, x1 + 1).color != NONE &&
           GetValue(y1, x1 + 1).handling == false) ||
          (GetValue(y2, x2 + 1).color != NONE &&
           GetValue(y2, x2 + 1).handling == false))) {
      SetValue(y1, x1, init_data);
      SetValue(y2, x2, init_data);
      SetValue(y1, ++x1, puyo1);
      SetValue(y2, ++x2, puyo2);
    }
  }

  void ControlRotateRight() {
    struct puyodata init_data;
    switch (puyorotate) {
      case 0:
        if (GetValue(y1, x1 + 1).color == NONE) {
          SetValue(y2, x2, init_data);
          SetValue(++y2, ++x2, puyo2);
          puyorotate = 1;
        } else if (GetValue(y1, x1 - 1).color == NONE) {
          SetValue(y2, x2, init_data);
          SetValue(y1, --x1, puyo1);
          SetValue(++y2, x2, puyo2);
          puyorotate = 1;
        }
        break;
      case 1:
        if (GetValue(y1 + 1, x1).color == NONE) {
          SetValue(y2, x2, init_data);
          SetValue(++y2, --x2, puyo2);
          puyorotate = 2;
        } else if (GetValue(y1 - 1, x1).color == NONE) {
          SetValue(y2, x2, init_data);
          SetValue(--y1, x1, puyo1);
          SetValue(y2, --x2, puyo2);
          puyorotate = 2;
        }
        break;
      case 2:
        if (GetValue(y1, x1 - 1).color == NONE) {
          SetValue(y2, x2, init_data);
          SetValue(--y2, --x2, puyo2);
          puyorotate = 3;
        } else if (GetValue(y1, x1 + 1).color == NONE) {
          SetValue(y2, x2, init_data);
          SetValue(y1, ++x1, puyo1);
          SetValue(--y2, x2, puyo2);
          puyorotate = 3;
        }
        break;
      case 3:
        if (GetValue(y1 - 1, x1).color == NONE) {
          SetValue(y2, x2, init_data);
          SetValue(--y2, ++x2, puyo2);
          puyorotate = 0;
        }
        break;
    }
  }

  void ControlRotateLeft() {
    struct puyodata init_data;
    switch (puyorotate) {
      case 0:
        if (GetValue(y1, x1 - 1).color == NONE) {
          SetValue(y2, x2, init_data);
          SetValue(++y2, --x2, puyo2);
          puyorotate = 3;
        } else if (GetValue(y1, x1 + 1).color == NONE) {
          SetValue(y2, x2, init_data);
          SetValue(y1, ++x1, puyo1);
          SetValue(++y2, x2, puyo2);
          puyorotate = 3;
        }
        break;
      case 1:
        if (GetValue(y1 - 1, x1).color == NONE) {
          SetValue(y2, x2, init_data);
          SetValue(--y2, --x2, puyo2);
          puyorotate = 0;
        }
        break;
      case 2:
        if (GetValue(y1, x1 + 1).color == NONE) {
          SetValue(y2, x2, init_data);
          SetValue(--y2, ++x2, puyo2);
          puyorotate = 1;
        } else if (GetValue(y1, x1 - 1).color == NONE) {
          SetValue(y2, x2, init_data);
          SetValue(y1, --x1, puyo1);
          SetValue(--y2, x2, puyo2);
          puyorotate = 1;
        }
        break;
      case 3:
        if (GetValue(y1 + 1, x1).color == NONE) {
          SetValue(y2, x2, init_data);
          SetValue(++y2, ++x2, puyo2);
          puyorotate = 2;
        } else if (GetValue(y1 - 1, x1).color == NONE) {
          SetValue(y2, x2, init_data);
          SetValue(--y1, x1, puyo1);
          SetValue(y2, ++x2, puyo2);
          puyorotate = 2;
        }
        break;
    }
  }

  bool FallPuyo() {
    bool all_landed = false;
    struct puyodata init_data;
    for (int y = GetLine() - 3; y >= 1; y--) {
      for (int x = 1; x < GetColumn() - 1; x++) {
        if (GetValue(y, x).color != NONE && GetValue(y + 1, x).color == NONE) {
          SetValue(y + 1, x, GetValue(y, x));
          SetValue(y, x, init_data);
          all_landed = true;
        }
      }
    }
    return all_landed;
  }

  bool LandingPuyo() {
    if ((GetValue(y1 + 1, x1).color != NONE &&
         GetValue(y1 + 1, x1).handling == false) ||
        (GetValue(y2 + 1, x2).color != NONE &&
         GetValue(y2 + 1, x2).handling == false)) {
      puyo1.handling = false;
      puyo2.handling = false;
      SetValue(y1, x1, puyo1);
      SetValue(y2, x2, puyo2);
      return true;
    }
    return false;
  }

  //ぷよ消滅処理を全座標で行う
  //消滅したぷよの数を返す
  int VanishPuyo() {
    int vanishednumber = 0;
    for (int y = 0; y < GetLine(); y++) {
      for (int x = 0; x < GetColumn(); x++) {
        vanishednumber += VanishPuyo(y, x);
      }
    }

    return vanishednumber;
  }

  //ぷよ消滅処理を座標(x,y)で行う
  //消滅したぷよの数を返す
  int VanishPuyo(unsigned int y, unsigned int x) {
    //判定個所にぷよがなければ処理終了
    if (GetValue(y, x).color == NONE || GetValue(y, x).color == WALL) {
      return 0;
    }

    //判定状態を表す列挙型
    // NOCHECK判定未実施，CHECKINGが判定対象，CHECKEDが判定済み
    enum checkstate { NOCHECK, CHECKING, CHECKED };

    //判定結果格納用の配列
    enum checkstate *field_array_check;
    field_array_check = new enum checkstate[GetLine() * GetColumn()];

    //配列初期化
    for (int i = 0; i < GetLine() * GetColumn(); i++) {
      field_array_check[i] = NOCHECK;
    }

    //座標(x,y)を判定対象にする
    field_array_check[y * GetColumn() + x] = CHECKING;

    //判定対象が1つもなくなるまで，判定対象の上下左右に同じ色のぷよがあるか確認し，あれば新たな判定対象にする
    bool checkagain = true;
    while (checkagain) {
      checkagain = false;

      for (int y = 0; y < GetLine(); y++) {
        for (int x = 0; x < GetColumn(); x++) {
          //(x,y)に判定対象がある場合
          if (field_array_check[y * GetColumn() + x] == CHECKING) {
            //(x+1,y)の判定
            if (x < GetColumn() - 1) {
              //(x+1,y)と(x,y)のぷよの色が同じで，(x+1,y)のぷよが判定未実施か確認
              if (GetValue(y, x + 1).color == GetValue(y, x).color &&
                  field_array_check[y * GetColumn() + (x + 1)] == NOCHECK) {
                //(x+1,y)を判定対象にする
                field_array_check[y * GetColumn() + (x + 1)] = CHECKING;
                checkagain = true;
              }
            }

            //(x-1,y)の判定
            if (x > 0) {
              if (GetValue(y, x - 1).color == GetValue(y, x).color &&
                  field_array_check[y * GetColumn() + (x - 1)] == NOCHECK) {
                field_array_check[y * GetColumn() + (x - 1)] = CHECKING;
                checkagain = true;
              }
            }

            //(x,y+1)の判定
            if (y < GetLine() - 1) {
              if (GetValue(y + 1, x).color == GetValue(y, x).color &&
                  field_array_check[(y + 1) * GetColumn() + x] == NOCHECK) {
                field_array_check[(y + 1) * GetColumn() + x] = CHECKING;
                checkagain = true;
              }
            }

            //(x,y-1)の判定
            if (y > 0) {
              if (GetValue(y - 1, x).color == GetValue(y, x).color &&
                  field_array_check[(y - 1) * GetColumn() + x] == NOCHECK) {
                field_array_check[(y - 1) * GetColumn() + x] = CHECKING;
                checkagain = true;
              }
            }

            //(x,y)を判定済みにする
            field_array_check[y * GetColumn() + x] = CHECKED;
          }
        }
      }
    }

    //判定済みの数をカウント
    int puyocount = 0;
    for (int i = 0; i < GetLine() * GetColumn(); i++) {
      if (field_array_check[i] == CHECKED) {
        puyocount++;
      }
    }

    // 4個以上あれば，判定済み座標のぷよを消す
    int vanishednumber = 0;
    if (4 <= puyocount) {
      for (int y = 0; y < GetLine(); y++) {
        for (int x = 0; x < GetColumn(); x++) {
          if (field_array_check[y * GetColumn() + x] == CHECKED) {
            struct puyodata init_data;
            SetValue(y, x, init_data);

            vanishednumber++;
          }
        }
      }
    }

    //メモリ解放
    delete[] field_array_check;

    return vanishednumber;
  }

  //落下予測地点を{y1, x1, y2, x2}の形で代入　　ぷよの色を{puyo1.color,
  // puyo2.color}の形で代入
  void SubsDropPointColor(unsigned int (&point_list)[4],
                          puyocolor (&color_list)[2]) {
    unsigned int y1_temp = y1, y2_temp = y2;
    if (puyorotate % 2 == 0) {
      do {
        y1_temp++;
        y2_temp++;
      } while (!((GetValue(y1_temp, x1).color != NONE &&
                  GetValue(y1_temp, x1).handling == false) ||
                 (GetValue(y2_temp, x2).color != NONE &&
                  GetValue(y2_temp, x2).handling == false)));
    } else {
      do {
        y1_temp++;
      } while (!(GetValue(y1_temp, x1).color != NONE &&
                 GetValue(y1_temp, x1).handling == false));

      do {
        y2_temp++;
      } while (!(GetValue(y2_temp, x2).color != NONE &&
                 GetValue(y2_temp, x2).handling == false));
    }

    point_list[0] = --y1_temp;
    point_list[1] = x1;
    point_list[2] = --y2_temp;
    point_list[3] = x2;
    color_list[0] = puyo1.color;
    color_list[1] = puyo2.color;
  }
};

// puyocolorによって色を設定する
void AttrsetColor(puyocolor color) {
  switch (color) {
    case NONE:
      attrset(COLOR_PAIR(5));
      break;
    case RED:
      attrset(COLOR_PAIR(1));
      break;
    case BLUE:
      attrset(COLOR_PAIR(2));
      break;
    case GREEN:
      attrset(COLOR_PAIR(3));
      break;
    case YELLOW:
      attrset(COLOR_PAIR(4));
      break;
    case WALL:
      attrset(COLOR_PAIR(5));
      break;
    default:
      break;
  }
}

void Display(FieldControl &field, bool land_flag, int chain, int score) {
  unsigned int field_line = field.GetLine(), field_column = field.GetColumn(),
               point_list[4];
  puyocolor color_list[2];

  for (int y = 2; y < field_line; y++) {
    for (int x = 0; x < field_column; x++) {
      switch (field.GetValue(y, x).color) {
        case NONE:
          attrset(COLOR_PAIR(5));
          mvaddch(y, x, ' ');
          break;
        case RED:
          attrset(COLOR_PAIR(1));
          mvaddch(y, x, 'O');
          break;
        case BLUE:
          attrset(COLOR_PAIR(2));
          mvaddch(y, x, 'O');
          break;
        case GREEN:
          attrset(COLOR_PAIR(3));
          mvaddch(y, x, 'O');
          break;
        case YELLOW:
          attrset(COLOR_PAIR(4));
          mvaddch(y, x, 'O');
          break;
        case WALL:
          attrset(COLOR_PAIR(5));
          mvaddch(y, x, '#');
          break;
        default:
          mvaddch(y, x, '?');
          break;
      }
    }
  }

  //ここにぷよがある状態で次のぷよが生成されるとゲームオーバーである印を描写
  if (field.GetValue(2, 3).color == NONE) {
    attrset(COLOR_PAIR(1));
    mvaddch(2, 3, 'X');
  }

  //ぷよの落下予測地点を描写
  if (!land_flag) {
    field.SubsDropPointColor(point_list, color_list);
    if (field.GetValue(point_list[0], point_list[1]).handling == false) {
      AttrsetColor(color_list[0]);
      mvaddch(point_list[0], point_list[1], '*');
    }
    if (field.GetValue(point_list[2], point_list[3]).handling == false) {
      AttrsetColor(color_list[1]);
      mvaddch(point_list[2], point_list[3], '*');
    }
  }

  //次と次の次のぷよを表示
  char next_str1[8], next_str2[8], next_str3[8], next_str4[8], next_str5[8],
      next_str6[8];
  sprintf(next_str1, "#####  ");
  sprintf(next_str2, "#   #  ");
  sprintf(next_str3, "#   ###");
  sprintf(next_str4, "###   #");
  sprintf(next_str5, "  #   #");
  sprintf(next_str6, "  #####");

  attrset(COLOR_PAIR(5));
  mvaddstr(2, 10, next_str1);
  mvaddstr(3, 10, next_str2);
  mvaddstr(4, 10, next_str3);
  mvaddstr(5, 10, next_str4);
  mvaddstr(6, 10, next_str5);
  mvaddstr(7, 10, next_str6);

  AttrsetColor(field.next_puyo_color[1]);
  mvaddch(3, 12, 'O');
  AttrsetColor(field.next_puyo_color[0]);
  mvaddch(4, 12, 'O');
  AttrsetColor(field.next_puyo_color[3]);
  mvaddch(5, 14, 'O');
  AttrsetColor(field.next_puyo_color[2]);
  mvaddch(6, 14, 'O');

  //スコアと連鎖数を表示
  char chain_str[256], score_str[256];
  attrset(COLOR_PAIR(5));
  sprintf(chain_str, "CHAIN : %d", chain);
  sprintf(score_str, "SCORE : %d", score);

  mvaddstr(9, 10, chain_str);
  mvaddstr(10, 10, score_str);

  refresh();
};

void GameOver() {
  clear();
  char msg1[256], msg2[256];
  sprintf(msg1, "GAME OVER!");
  sprintf(msg2, "Press q to quit this game.");
  mvaddstr(0, 0, msg1);
  mvaddstr(1, 0, msg2);
  refresh();
  int ch;
  do {
    ch = getch();
  } while (ch != 'q');
};

// scoreを返す
int ReturnScore(int chain, int vanishnum) {
  //連鎖ボーナスの配列
  int chain_score_array[19] = {1,   8,   16,  32,  64,  96,  128, 160, 192, 224,
                               256, 288, 320, 352, 384, 416, 448, 480, 512};
  if (chain >= 0 && chain <= 19) {
    return vanishnum * chain_score_array[chain - 1] * 10;
  }
  return 0;
}

int main(void) {
  initscr();
  start_color();
  noecho();
  cbreak();
  curs_set(0);
  keypad(stdscr, TRUE);
  timeout(0);

  srand((unsigned int)time(NULL));

  init_pair(1, COLOR_RED, COLOR_BLACK);
  init_pair(2, COLOR_BLUE, COLOR_BLACK);
  init_pair(3, COLOR_GREEN, COLOR_BLACK);
  init_pair(4, COLOR_YELLOW, COLOR_BLACK);
  init_pair(5, COLOR_WHITE, COLOR_BLACK);

  FieldControl field;

  field.GeneratePuyo();

  int delay = 0, waitCount = 60000, score = 0, chain = 0;

  while (1) {
    int ch;
    ch = getch();
    bool land_flag = false;

    switch (ch) {
      case KEY_LEFT:
        field.ControlLeft();
        break;
      case KEY_RIGHT:
        field.ControlRight();
        break;
      case 'z':
        field.ControlRotateLeft();
        break;
      case 'x':
        field.ControlRotateRight();
        break;
      case KEY_DOWN:
        land_flag = field.ControlDown();
        break;
      case 'p':
        int ch_temp;
        char pause_str[256];
        sprintf(pause_str, "PAUSE");
        mvaddstr(12, 10, pause_str);
        do {
          ch_temp = getch();
        } while (ch_temp != 'p');
        sprintf(pause_str, "     ");
        mvaddstr(12, 10, pause_str);
        break;
      default:
        break;
    }

    if (delay % waitCount == 0 || land_flag) {
      if (field.LandingPuyo()) {
        int vanishnum;
        land_flag = true;
        // napms(ms)  msミリ秒間スリープ
        do {
          while (field.FallPuyo()) {
            Display(field, land_flag, chain, score);
            napms(100);
          }
          if (vanishnum = field.VanishPuyo()) {
            chain++;
            score += ReturnScore(chain, vanishnum);
            Display(field, land_flag, chain, score);
            napms(800);
          }
        } while (vanishnum);
      }

      if (land_flag) {
        if (!(field.GeneratePuyo())) {
          GameOver();
          break;
        }
        while (getch() != EOF) {
        }
      } else {
        field.ControlDown();
      }
    }
    delay++;
    chain = 0;
    Display(field, land_flag, chain, score);
  }

  endwin();

  return 0;
}
