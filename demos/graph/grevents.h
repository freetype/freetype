#ifndef GREVENTS_H
#define GREVENTS_H


#define gr_event_none  0
#define gr_event_wait  1
#define gr_event_poll  2
#define gr_event_flush 3

#define gr_mouse_down  0x04
#define gr_mouse_move  0x08
#define gr_mouse_up    0x10
#define gr_mouse_drag  0x20

#define gr_key_down 0x40
#define gr_key_up   0x80


#define gr_event_mouse 0x3C
#define gr_event_key   0xC0

#define gr_event_type  (gr_event_mouse | gr_event_key)


  typedef enum grKey_
  {
    grKeyNone = 0,

    grKeyF1,
    grKeyF2,
    grKeyF3,
    grKeyF4,
    grKeyF5,
    grKeyF6,
    grKeyF7,
    grKeyF8,
    grKeyF9,
    grKeyF10,
    grKeyF11,
    grKeyF12,

    grKeyLeft,
    grKeyRight,
    grKeyUp,
    grKeyDown,

    grKeyIns,
    grKeyDel,
    grKeyHome,
    grKeyEnd,
    grKeyPageUp,
    grKeyPageDown,

    grKeyEsc,
    grKeyTab,
    grKeyBackSpace,
    grKeyReturn,

    grKeyMax,
    grKeyForceShort = 0x7FFF  /* this forces the grKey to be stored */
                              /* on at least one short !            */

  } grKey;

#define  grKEY(c)    ((grKey)(c))

#define  grKeyAlt    ((grKey)0x8000)
#define  grKeyCtrl   ((grKey)0x4000)
#define  grKeyShift  ((grKey)0x2000)

#define  grKeyModifiers ((grKey)0xE000)

#define  grKey0       grKEY('0')
#define  grKey1       grKEY('1')
#define  grKey2       grKEY('2')
#define  grKey3       grKEY('3')
#define  grKey4       grKEY('4')
#define  grKey5       grKEY('5')
#define  grKey6       grKEY('6')
#define  grKey7       grKEY('7')
#define  grKey8       grKEY('8')
#define  grKey9       grKEY('9')


#define  grKeyPlus        grKEY('+')
#define  grKeyLess        grKEY('-')
#define  grKeyEqual       grKEY('=')
#define  grKeyMult        grKEY('*')
#define  grKeyDollar      grKEY('$')
#define  grKeySmaller     grKEY('<')
#define  grKeyGreater     grKEY('>')
#define  grKeyQuestion    grKEY('?')
#define  grKeyComma       grKEY(',')
#define  grKeyDot         grKEY('.')
#define  grKeySemiColumn  grKEY(';')
#define  grKeyColumn      grKEY(':')
#define  grKeyDiv         grKEY('/')
#define  grKeyExclam      grKEY('!')
#define  grKeyPercent     grKEY('%')
#define  grKeyLeftParen   grKEY('(')
#define  grKeyRightParen  grKEY('(')
#define  grKeyAt          grKEY('@')
#define  grKeyUnder       grKEY('_')


  typedef struct grEvent_
  {
    int    type;
    grKey  key;
    int    x, y;

  } grEvent;



#endif /* GREVENTS_H */

