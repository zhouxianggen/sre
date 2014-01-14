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
#include "common.h"

namespace ucltp {

// supported wildcards
enum token_t {
	NUMBER, // \d 0	
	SPACE, // \s 1
	ALPHABET, // \a 2
	CHINESE, // \c 3
	NOT_NUMBER,	// \D 4
	NOT_SPACE, // \S 5
	NOT_ALPHABET,	// \A 6
	NOT_CHINESE, // \C 7
	POSI_BEGIN, // ^ 8
	POSI_END, // $ 9
	LEFT_PARENTHESE, // ( 10
	RIGHT_PARENTHESE, // ) 11
	BACKTRACE, // | 12
	DOT, // . 13
	REPEAT_ONCE_ZERO, // ? 14
	REPEAT_MORE_ZERO, // * 15
	REPEAT_MORE_ONCE,	// + 16
	REPEAT_ZERO_ONCE, // ?? 17
	REPEAT_ZERO_MORE, // *?	18
	REPEAT_ONCE_MORE, // +? 19
	COMMON, // common 20
	END_REGEXP, // 0 21
	EMPTY // null 22
};

/* graph node */
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

struct graph_t {
	node_t *_first;
	node_t *_last;
	
  graph_t(node_t *pf=0, node_t *pl=0): _first(pf), _last(pl) {}
	bool empty() { return _first == NULL; }
	bool single() { return _first == _last; }
};

class	Sre {
public:
	Sre () {}
	bool compile(const char *st);
	int match(const vector<char_t>& chars, int start);
	~Sre () { release(); }
private:
	graph_t _graph;
	vector<node_t*> _collector;
	
  void release();
  token_t get_token(const char *str, int &pos, uint32& code);
	graph_t make_graph(const char *str, int start, int end, graph_t prev);
	int	match_graph (const vector<char_t>& chars, int posi, node_t* pnode);
	node_t* alloc_node (int token, uint32 code=0);
	node_t* alloc_node (const node_t &n);
};

}
#endif
