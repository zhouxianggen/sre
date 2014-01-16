/*
 * sre: simple regular engine
 * Copyright (C) 2014 UC
 * Author: zhouxg@ucweb.com
 *
 */
#ifndef UCLTP_SRE_H
#define UCLTP_SRE_H

#include <stdio.h>
#include <vector>
using std::vector;
using std::pair;
#include "def.h"

namespace ucltp {

// supported wildcards
enum token_t {
  DIGIT, // \d 0
  NUMBER, // \n 1
  SPACE, // \s 2
  ALPHABET, // \a 3
  CHINESE, // \c 4
  NOT_DIGIT,  // \D 5
  NOT_NUMBER, // \N 6
  NOT_SPACE, // \S 7
  NOT_ALPHABET,  // \A 8
  NOT_CHINESE, // \C 9
  POSI_BEGIN, // ^ 10
  POSI_END, // $ 11
  LEFT_PARENTHESE, // ( 12
  RIGHT_PARENTHESE, // ) 13
  BACKTRACE, // | 14
  DOT, // . 15
  REPEAT_ONCE_ZERO, // ? 16
  REPEAT_MORE_ZERO, // * 17
  REPEAT_MORE_ONCE,  // + 18
  REPEAT_ZERO_ONCE, // ?? 19
  REPEAT_ZERO_MORE, // *?  20
  REPEAT_ONCE_MORE, // +? 21
  COMMON, // common 22
  END_REGEXP, // 0 23
  EMPTY // null 24
};

// graph node
struct node_t {
  int _token;
  uint32 _code;
  vector<node_t*> _nexts;
  bool _show;
  
  node_t(int tok, uint32 code=0) : _token(tok), _code(code), _show(false) {}
  void add_next(node_t* p) { _nexts.push_back(p); }
  
  void show(int lev=0) {
    for (int i=0; i<lev; i+=1) printf("--");
    printf("%x(%2d,%4x)\n", (uint64)this, _token, _code);
    if (_show == false) {
      _show = true;
      for (int i=0; i<_nexts.size(); i+=1)
        _nexts[i]->show(lev+1);
    }
  }
};

// graph
struct graph_t {
  node_t *_first;
  node_t *_last;
  
  graph_t(node_t *pf=0, node_t *pl=0): _first(pf), _last(pl) {}
  bool empty() { return _first == NULL; }
  bool single() { return _first == _last; }
};

class SreImpl {
public:
  SreImpl() {}
  bool compile(const char *st);
  int match(const vector<char_t>& chars, int start);
  ~SreImpl() { release(); }
private:
  graph_t _graph;
  vector<node_t*> _collector;
  
  void release();
  token_t get_token(const char *str, int &pos, uint32& code);
  graph_t make_graph(const char *str, int start, int end, graph_t prev);
  int  match_graph (const vector<char_t>& chars, int posi, node_t* pnode);
  node_t* alloc_node(int token, uint32 code=0);
  node_t* alloc_node(const node_t &n);
};

// wrapper
class Sre {
public:
  int build(const char* fre);
  match_result_t match(const vector<char_t>& chars, int start);
  ~Sre () {
    for (int i=0; i<_objects.size(); i+=1)
      delete _objects[i].first;
  }
private:
  vector<pair<SreImpl*, int> > _objects;
};

}
#endif
