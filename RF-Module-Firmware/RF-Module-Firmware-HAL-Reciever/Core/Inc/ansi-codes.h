/*
 * ansi-codes.h
 *
 *  Created on: Dec 29, 2024
 *      Author: abina
 */

#ifndef INC_ANSI_CODES_H_
#define INC_ANSI_CODES_H_

#define ESC "\x1b"

#define SCR_CLR2END  ESC "[0J" // Clear to end of screen
#define SCR_CLR2BEG  ESC "[1J" // Clear to begining of screen
#define SCR_CLR_ALL  ESC "[2J" // Clear entire screen
#define SCR_CLR_SAV  ESC "[3J" // Clear saved lines
#define SCR_CLR2LEND ESC "[0K" // Clear to end of line
#define SCR_CLR2LBEG ESC "[1K" // Clear to begining of line
#define SCR_CLR_LINE ESC "[1K" // Clear entire line
#define SCR_RESTORE  ESC "[?47l"
#define SCR_SAVE     ESC "[?47h"

#define CUR_HOME         ESC "[H"
#define CUR_SET(l, c)    ESC "["#l";"#c"H"
#define DYN_CUR_SET      CUR_SET(%d, %d)
#define CUR_UP(n)        ESC "["#n"A"
#define DYN_CUR_UP       CUR_UP(%d)
#define CUR_DOWN(n)      ESC "["#n"B"
#define DYN_CUR_DOWN     CUR_DOWN(%d)
#define CUR_RIGHT(n)     ESC "["#n"C"
#define DYN_CUR_RIGHT    CUR_RIGHT(%d)
#define CUR_LEFT(n)      ESC "["#n"D"
#define DYN_CUR_LEFT     CUR_LEFT(%d)
#define CUR_UP_BEG(n)    ESC "["#n"E"
#define DYN_CUR_UP_BEG   CUR_UP_BEG(%d)
#define CUR_DOWN_BEG(n)  ESC "["#n"F"
#define DYN_CUR_DOWN_BEG CUR_DOWN_BEG(%d)
#define CUR_SET_COL(n)   ESC "["#n"G"
#define DYN_CUR_SET_COL  CUR_SET_COL(%d)
#define CUR_REQ_POS      ESC "[6n"
#define CUR_MOVE_1UP     ESC "M"
#define CUR_SAVE_DEC     ESC "7"
#define CUR_RESTORE_DEC  ESC "8"
#define CUR_SAVE_SCO     ESC "[s"
#define CUR_RESTORE_SCO  ESC "[u"
#define CUR_INVISIBLE    ESC "[?25l"
#define CUR_VISIBLE      ESC "[?25h"

#define GR_SEQ(s) ESC"[" s "m"

/* Text colors */
#define FG_BLACK    ";30"
#define FG_RED      ";31"
#define FG_GREEN    ";32"
#define FG_YELLOW   ";33"
#define FG_BLUE     ";34"
#define FG_MAGENTA  ";35"
#define FG_CYAN     ";36"
#define FG_WHITE    ";37"

#define __SETFG_BLACK   ESC "[" FG_BLACK
#define __SETFG_RED     ESC "[" FG_RED
#define __SETFG_GREEN   ESC "[" FG_GREEN
#define __SETFG_YELLOW  ESC "[" FG_YELLOW
#define __SETFG_BLUE    ESC "[" FG_BLUE
#define __SETFG_MAGENTA ESC "[" FG_MAGENTA
#define __SETFG_CYAN    ESC "[" FG_CYAN
#define __SETFG_WHITE   ESC "[" FG_WHITE

#define SETFG_BLACK   __SETFG_BLACK   "m"
#define SETFG_RED     __SETFG_RED     "m"
#define SETFG_GREEN   __SETFG_GREEN   "m"
#define SETFG_YELLOW  __SETFG_YELLOW  "m"
#define SETFG_BLUE    __SETFG_BLUE    "m"
#define SETFG_MAGENTA __SETFG_MAGENTA "m"
#define SETFG_CYAN    __SETFG_CYAN    "m"
#define SETFG_WHITE   __SETFG_WHITE   "m"

#define BRIGHT_FG(c)     FG_##c ";1"
#define FG256(n)         ";38;5;" #n
#define SETBRIGHT_FG(c)  ESC "[" BRIGHT_FG(c) "m"
#define SETFG256(n)      ESC "[" FG256(n) "m"
#define DYN_BRIGHT_FG    BRIGHT_FG(%d)
#define DYN_FG256        FG256(%d)
#define DYN_SETBRIGHT_FG SETBRIGHT_FG(%d)
#define DYN_SETFG256     SETFG256(%d)

#define FG_RGB(r, g, b)    ";38;2;"#r";"#g";"#b
#define SETFG_RGB(r, g, b) ESC "[" FG_RGB(r, g, b) "m"
#define DYN_FG_RGB         FG_RGB(%d, %d, %d)
#define DYN_SETFG_RGB      SETFG_RGB(%d, %d, %d)

#define FG_DEFAULT ESC "[39m" // Set the foreground
// color to the default one
// (i.e. reset foreground
//  colof only)

/* Background colors */
#define BG_BLACK    ";40"
#define BG_RED      ";41"
#define BG_GREEN    ";42"
#define BG_YELLOW   ";43"
#define BG_BLUE     ";44"
#define BG_MAGENTA  ";45"
#define BG_CYAN     ";46"
#define BG_WHITE    ";47"

#define __SETBG_BLACK   ESC "[" BG_BLACK
#define __SETBG_RED     ESC "[" BG_RED
#define __SETBG_GREEN   ESC "[" BG_GREEN
#define __SETBG_YELLOW  ESC "[" BG_YELLOW
#define __SETBG_BLUE    ESC "[" BG_BLUE
#define __SETBG_MAGENTA ESC "[" BG_MAGENTA
#define __SETBG_CYAN    ESC "[" BG_CYAN
#define __SETBG_WHITE   ESC "[" BG_WHITE

#define SETBG_BLACK   __SETBG_BLACK   "m"
#define SETBG_RED     __SETBG_RED     "m"
#define SETBG_GREEN   __SETBG_GREEN   "m"
#define SETBG_YELLOW  __SETBG_YELLOW  "m"
#define SETBG_BLUE    __SETBG_BLUE    "m"
#define SETBG_MAGENTA __SETBG_MAGENTA "m"
#define SETBG_CYAN    __SETBG_CYAN    "m"
#define SETBG_WHITE   __SETBG_WHITE   "m"

#define BRIGHT_BG(c)     BG_##c ";1"
#define BG256(n)         ";48;5;" #n
#define SETBRIGHT_BG(c)  ESC "[" BRIGHT_BG(c) "m"
#define SETBG256(n)      ESC "[" BG256(n) "m"
#define DYN_BRIGHT_BG    BRIGHT_BG(%d)
#define DYN_BG256        BG256(%d)
#define DYN_SETBRIGHT_BG SETBRIGHT_BG(%d)
#define DYN_SETBG256     SETBG256(%d)

#define BG_RGB(r, g, b)    ";48;2;"#r";"#g";"#b
#define SETBG_RGB(r, g, b) ESC "[" BG_RGB(r, g, b) "m"
#define DYN_BG_RGB         BG_RGB(%d, %d, %d)
#define DYN_SETBG_RGB      SETBG_RGB(%d, %d, %d)

#define BG_DEFAULT ESC "[49m" // Set the background color
// to the default one
// (i.e. reset background
//  color only)

/* Text styles */
#define TXT_BLD ";1" // bold
#define TXT_DIM ";2" // dim/faint
#define TXT_ITL ";3" // italic
#define TXT_UNL ";4" // underline
#define TXT_BLK ";5" // blinking mode (idk what it does)
#define TXT_REV ";7" // reversed
#define TXT_HID ";8" // hidden/invisible
#define TXT_STK ";9" // strikethrough

#define SETTXT_BLD ESC "[" TXT_BLD "m"
#define SETTXT_DIM ESC "[" TXT_DIM "m"
#define SETTXT_ITL ESC "[" TXT_ITL "m"
#define SETTXT_UNL ESC "[" TXT_UNL "m"
#define SETTXT_BLK ESC "[" TXT_BLK "m"
#define SETTXT_REV ESC "[" TXT_REV "m"
#define SETTXT_HID ESC "[" TXT_HID "m"
#define SETTXT_STK ESC "[" TXT_STK "m"

#define GR_RESET   ESC "[" "0m"

#define SCR_CLR2END  ESC "[0J" // Clear to end of screen
#define SCR_CLR2BEG  ESC "[1J" // Clear to begining of screen
#define SCR_CLR_ALL  ESC "[2J" // Clear entire screen
#define SCR_CLR_SAV  ESC "[3J" // Clear saved lines
#define SCR_CLR2LEND ESC "[0K" // Clear to end of line
#define SCR_CLR2LBEG ESC "[1K" // Clear to begining of line
#define SCR_CLR_LINE ESC "[1K" // Clear entire line
#define SCR_RESTORE  ESC "[?47l"
#define SCR_SAVE     ESC "[?47h"

#endif /* INC_ANSI_CODES_H_ */
