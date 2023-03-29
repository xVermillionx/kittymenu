#include <stdio.h>
#include <ncurses.h>

// #define WIN_PADDING 4
#ifdef WIN_PADDING
  #define WIN_PADDING_RIGHT WIN_PADDING
  #define WIN_PADDING_TOP WIN_PADDING
#endif
#define WIN_PADDING_RIGHT 2
#define WIN_PADDING_TOP 1

#define PADDING 2

#define WIDTH 80
#define HEIGHT 20

#define AUTOHEIGHT true

#ifdef AUTOHEIGHT
  #define HEIGHT n_choices
#endif

#define CENTER_WINDOW false
#define FULL_WINDOW true
#define WIN_TOP 2
#define WIN_RIGHT 2

#define LEFT 0
#define MIDDLE 1
#define RIGHT 2

#define ALIGNMENT LEFT

#define MENU_CHOICE " %s "

#if FULL_WINDOW
  #define WIN_TOP WIN_PADDING_TOP
  #define WIN_RIGHT WIN_PADDING_RIGHT
  #define WIDTH col - WIN_PADDING_RIGHT*2
  #define HEIGHT row-4 - WIN_PADDING_TOP*2
#else if 

#endif

const char* title = "Switcher";

int startx = 0;
int starty = 0;

int row = 0;
int col = 0;

#include <functional>
#include <iostream>
#include <cstring>
#include <chrono>
#include <thread>

#include <sstream>
#include <json/json.h>
#include <json/reader.h>
#include <json/writer.h>
#include <json/value.h>
#include "runUnixCMD.h"

class choice{
public:
  int id;
  const char* name;
  std::function<int(choice&, int c)> exec;
  choice() {
    // std::cout << "Created: " << this << std::endl;
  };
  choice(int _id, const char *_name, std::function<int(choice &, int c)> _exec) : id(_id), name(_name), exec(_exec){
    // std::cout << "CreatedDiff: " << this << std::endl;
  };
  choice(const choice &p) : id(p.id), name(p.name), exec(p.exec){
    // std::cout << "Copied: " << "from: " << &p << " " << "to: " << this << std::endl;
  };
  // choice(const choice &&p) : id(p.id), name(p.name), exec(p.exec){
  //   // std::cout << "Moves: " << "from: " << &p << " " << "to: " << this << std::endl;
  // };
  ~choice(){
    // std::cout<<"Destroyed: "<<this<<std::endl;
  }
  int execr(int c){
    exec(*this, c);
  }
};
// choice mexit{3, "Exit", ([&](choice &that) -> int { std::cout << that.id << ": " << that.name << std::endl; waitasec(); return that.id; })};
// auto f_x =    ([&](choice& that)->int{ std::cout << that.id << that.name << std::endl; waitasec(); return that.id;});

void waitasec(int i = 1){
  std::this_thread::sleep_for(std::chrono::milliseconds(1000*i));
}

choice *choices = nullptr;
int n_choices;
// choice choices[] = {
//     choice{0, "Test0", ([&](choice &that) -> int { std::cout << that.id << ": " << that.name << std::endl; waitasec(); return that.id; })},
//     choice{1, "Test1", ([&](choice &that) -> int { std::cout << that.id << ": " << that.name << std::endl; waitasec(); return that.id; })},
//     choice{2, "Test2", ([&](choice &that) -> int { std::cout << that.id << ": " << that.name << std::endl; waitasec(); return that.id; })},
//     choice{3, "Close",  ([&](choice &that) -> int { std::cout << that.id << ": " << that.name << std::endl; waitasec(); return that.id; })},
// };

//int n_choices = sizeof(choices) / sizeof(choices[0]);

void print_menu(WINDOW *menu_win, int highlight);

int main()
{

  std::cout << "\033]0;" << title << "\007" << std::flush;

  Json::Value root;
	Json::Reader reader;
	std::string kittyjson = runUnixCommandAndCaptureOutput("kitty @ ls");
  if(kittyjson == ""){
    std::cout << "\n\tallow_remote_control yes\n" << std::endl;
    return 1;
  }
	std::string gotomatch[] = { "id", "title", "pid", "cwd", "cmdline", "num", "env" };

	bool parsingSuccessful = reader.parse(kittyjson.c_str(), root); //parse process
	if (!parsingSuccessful)
	{
		std::cout << "Failed to parse"
							<< reader.getFormattedErrorMessages();
		return 0;
	}

	
int highlight = 1;

	for (const auto &x : root)
	{
    n_choices = x["tabs"].size();
		choices = new choice[n_choices];

    for (const auto &y : x["tabs"])
		{
      static int count = 0;
			std::stringstream title;
      std::stringstream cmd;
      std::stringstream cmd2;

      // title << "(" << count << ")\t" << y["id"].asInt() << ": " << y["title"].asString() << (y["is_focused"].asBool() ? "*" : "");
      title << "(" << count << ")\t" << y["id"] << ": ";

      // title << y["title"].asString();

      for (auto z = y["windows"].begin(); z != y["windows"].end(); z++)
      {
        title << (*z)["title"].asString();
        if((std::next(z)) != y["windows"].end()) title << " | ";
      }

      title << (y["is_focused"].asBool() ? " * " : " ") << y["windows"].size() << " windows";
      // cmd << "kitty @ resize-window -m " << gotomatch[2] << ":" << y["windows"][0][gotomatch[2]];
      cmd << "kitty @ focus-window -m " << gotomatch[2] << ":" << y["windows"][0][gotomatch[2]];
      cmd2 << "kitty @ close-tab -m " << gotomatch[2] << ":" << y["windows"][0][gotomatch[2]];

      count++;
      if (y["is_focused"].asBool())
      {
        highlight = count;
      }

      std::string titlestr = title.str();
      std::string cmdstr = cmd.str();
      std::string cmdstr2 = cmd2.str();
/*
      std::string xcmdstr = std::string("") + cmdstr;
      char *cmdcstr = new char[xcmdstr.length()];
      strcpy(cmdcstr, xcmdstr.c_str()); */

      char *cmdcstr = new char[cmdstr.length()];
      strcpy(cmdcstr, cmdstr.c_str());
      char *cmdcstr2 = new char[cmdstr2.length()];
      strcpy(cmdcstr2, cmdstr2.c_str());
      char * titlecstr = new char[titlestr.length()];
      strcpy(titlecstr, titlestr.c_str());

      *choices = choice(reinterpret_cast<int>(y["id"].asInt()),
                        reinterpret_cast<const char *>(titlecstr),
                        // ([&, cmdstr, cmdstr2](choice &that, int c) -> int {
                        ([=](choice &that, int c) -> int {
                          if(c == 'x'){
                            // std::cout << (cmdcstr2) << std::endl; waitasec(5);
                            runUnixCommandAndCaptureOutput(cmdcstr2); /*  waitasec(0); */
                          }
                          else{
                            // std::cout << (cmdcstr) << std::endl; waitasec(5);
                            runUnixCommandAndCaptureOutput(cmdcstr); /*  waitasec(0); */
                          }
                          return that.id;
                        }));

      choices++;
    }
  choices = choices - n_choices;
  }

  // for(int i = 0; i < n_choices; i++){
  //   std::cout << (*choices).id << "\n: " << (*choices).name << "\n---------------------\n\n" << std::endl;
  //   choices++;
  // }
  // choices-=n_choices;

  // std::cin.get();


  WINDOW *menu_win;

	// int highlight = 1; // defined further up to catch what window is focused
  
	int choiceI = 0;
	int c;

	initscr();
  curs_set(0);
	clear();
	noecho();
	cbreak();	/* Line buffering disabled. pass on everything */
  // int row = 0; made global
  // int col = 0; made global
  getmaxyx(stdscr, row, col);
  if(CENTER_WINDOW){
    startx = (col - WIDTH) / 2;
    starty = (row - HEIGHT) / 2;
  }
  else if(FULL_WINDOW){
    startx = WIN_RIGHT;
    starty = WIN_TOP;
  }
  else{
    startx = WIN_RIGHT;
    starty = WIN_TOP;
  }



  menu_win = newwin(HEIGHT+PADDING*2, WIDTH, starty, startx);
	keypad(menu_win, TRUE);
	// mvprintw(0, 0, "Use arrow keys to go up and down, Press enter to select a choice");
	refresh();
	print_menu(menu_win, highlight);
	while(1)
	{
    static int oldcol = col;
    static int oldrow = row;
    getmaxyx(stdscr, row, col);
    
    if(oldcol != col || oldrow != row){
      if(CENTER_WINDOW){
        startx = (col - WIDTH) / 2;
        starty = (row - HEIGHT) / 2;
      }
      else if(FULL_WINDOW){
        startx = WIN_RIGHT;
        starty = WIN_TOP;
      }
      else{
        startx = WIN_RIGHT;
        starty = WIN_TOP;
      }
      menu_win = newwin(HEIGHT+PADDING*2, WIDTH, starty, startx);
      keypad(menu_win, TRUE);
	    clear();
      refresh();
      print_menu(menu_win, highlight);
    }
    c = wgetch(menu_win);
    // std::cout << c << std::endl;
    if (c == KEY_UP){
      if (highlight == 1) //Loop around
        highlight = n_choices;
      else
        --highlight;
    }
    else if(c == KEY_DOWN){
      if (highlight == n_choices) //Loop around
        highlight = 1;
      else
        ++highlight;
    }
    else if (c == 10)
    {
      choiceI = highlight;
    }
    else if (c < 58 && c > 48)
    {
      int num = c-48;
      if(num<=n_choices){
        highlight = num;
      }
    }
    else if (c == 'g')
    {
      static bool once = false;
      if(once){
        highlight = 1;
        once = false;
      }
      else{
        once = true;
      }
    }
    else if (c == 'G')
    {
      highlight = n_choices;
    }
    else if (c == 'x')
    {
      choices[highlight - 1].execr(c);
      n_choices--;
      if(highlight < n_choices){
        for (int n = highlight-1 ; n < n_choices; n++)
        {
          choices[n] = choices[n+1];
        }
      }

      //Highlight prev
      if(highlight > 1){
        highlight--;
      }

      refresh();
    }
    else{
      mvprintw(row, 0, "Character pressed is = %3d Hopefully it can be printed as '%c'", c, c);
      refresh();
    }

		print_menu(menu_win, highlight);
		if(choiceI != 0)	/* User did a choice come out of the infinite loop */
			break;
	}	
	// mvprintw(row - 1, 0, "You chose choice %d with choice string %s\n", choice, choices[choice - 1].name);
  move(0, 0);
  choices[choiceI - 1].execr(c);
  clrtoeol();
  refresh();
	endwin();
  curs_set(1);
	return 0;
}


void print_menu(WINDOW *menu_win, int highlight)
{
	int x, y, i;	

	x = 2;
	y = PADDING;
	box(menu_win, 0, 0);
	for(i = 0; i < n_choices; ++i)
	{	if(highlight == i + 1) /* High light the present choice */
		{	wattron(menu_win, A_REVERSE);
      switch(ALIGNMENT){
      case LEFT:
        mvwprintw(menu_win, y, int(PADDING), MENU_CHOICE, choices[i].name);
        break;
      case RIGHT:
	  		mvwprintw(menu_win, y, int(WIDTH-PADDING-strlen(choices[i].name)), MENU_CHOICE, choices[i].name);
        break;
      case MIDDLE:
  			mvwprintw(menu_win, y, int((WIDTH-strlen(choices[i].name))/2), MENU_CHOICE, choices[i].name);
        break;
      default:
        mvwprintw(menu_win, y, int((WIDTH-strlen(choices[i].name))/2), MENU_CHOICE, choices[i].name);
        break;
      };
			wattroff(menu_win, A_REVERSE);
		}
		else
      switch(ALIGNMENT){
      case LEFT:
        mvwprintw(menu_win, y, int(PADDING), MENU_CHOICE, choices[i].name);
        break;
      case RIGHT:
	  		mvwprintw(menu_win, y, int(WIDTH-PADDING-strlen(choices[i].name)), MENU_CHOICE, choices[i].name);
        break;
      case MIDDLE:
  			mvwprintw(menu_win, y, int((WIDTH-strlen(choices[i].name))/2), MENU_CHOICE, choices[i].name);
        break;
      default:
        mvwprintw(menu_win, y, int((WIDTH-strlen(choices[i].name))/2), MENU_CHOICE, choices[i].name);
        break;
      };
		++y;
	}
	wrefresh(menu_win);
}
