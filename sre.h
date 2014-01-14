/*
 * sre: simple regular engine
 * Copyright (C) 2012 UC
 * Author: zhouxg@ucweb.com
 *
 */
#ifndef SRE_H
#define SRE_H
#include <string>
#include <vector>
using namespace std;
namespace UCLTP {

#define MAX_U8C_LENGTH 7
#define MAX_PARENTHESE_CNT 32

enum Token {
	/* supported wildcards */
	NUMBER, // \d	
	SPACE, // \s
	ALPHABET, // \a
	CHINESE, // \c
	NOT_NUMBER,	// \D
	NOT_SPACE, // \S
	NOT_ALPHABET,	// \A
	NOT_CHINESE, // \C
	POSI_BEGIN, // ^
	POSI_END, // $
	LEFT_PARENTHESE, // (
	RIGHT_PARENTHESE, // )
	BACKTRACE, // |
	DOT, // .
	REPEAT_ONCE_ZERO, // ?
	REPEAT_MORE_ZERO, // *
	REPEAT_MORE_ONCE,	// +
	REPEAT_ZERO_ONCE, // ??
	REPEAT_ZERO_MORE, // *?	
	REPEAT_ONCE_MORE, // +?
	COMMON, // common
	END_REGEXP, // 0
	EMPTY // null
};

/* graph node */
struct Node {
	int token;
	char u8c[MAX_U8C_LENGTH];
	int	ord;
	int	nnexts;
	Node **pnexts;
	Node (int t, char *u=NULL, int r=0);
	
	bool add_next (Node* p);
	void release ();
};

struct	Graph {
	Graph (Node *pf=0, Node *pl=0): pfirst(pf), plast(pl) {}
	bool empty () { return pfirst == NULL; }
	bool single () { return pfirst == plast; }
	Node *pfirst;
	Node *plast;
};

class	Sre {
public:
	Sre ();
	enum Flags { IGNORECASE=0x1, DOTALL=0x02, NONE=0 };
	bool compile (const char *st, int flag=NONE);
	int match (const char *st);
	const char* search (const char *st);
	string group (const char *st, int ord);
	~Sre ();
private:
	int get_u8char (const char *st, char *u8c);
	Token get_token (const char *st, int &pos, char *u8c);
	Graph make_graph (const char *st, int s, int e, Graph prev);
	int	match_graph (const char *st, int posi, Node* pnode);
	Node* alloc_node (int token, char *u8c=NULL, int ord=0);
	Node* alloc_node (const Node &n);
	void release ();
private:
	int flags;
	int parenthese_cnt;
	int	parenthese_ord;
	int begins[MAX_PARENTHESE_CNT];
	int ends[MAX_PARENTHESE_CNT];
	Graph graph;
	vector<Node*> collector;
};

}
#endif
