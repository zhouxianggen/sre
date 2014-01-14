#include <iostream>
using namespace std;
#include "strkit.h"
#include "sre.h"
namespace UCLTP {
	
Node::Node (int t, char *u, int r)
:token(t), ord(r), nnexts(0), pnexts(0)
{
	u8c[MAX_U8C_LENGTH-1] = 0;
	if (u)
		for (int i=0; i<MAX_U8C_LENGTH && u[i]; i+=1)
			u8c[i] = u[i];
}

bool Node::add_next (Node *p)
{
	Node** pnew = (Node**)malloc((nnexts+1) * sizeof(Node*));
	if (!pnew)
		return false;
	memcpy(pnew, pnexts, nnexts*sizeof(Node*));
	pnew[nnexts++] = p;
	free(pnexts);
	pnexts = pnew;
	return true;
}

void Node::release()
{
	free(pnexts);
	nnexts = 0;
	pnexts = NULL;
}	

Sre::Sre ()
:flags(NONE), parenthese_cnt(0), parenthese_ord(0)
{
}

Sre::~Sre()
{
	release();
}

void Sre::release ()
{
	parenthese_cnt = 0;
	parenthese_ord = 0;
	for (int i=0; i<collector.size(); i++) {
		collector[i]->release();
		delete(collector[i]);
	}
	collector.clear();
}

bool Sre::compile (const char* st, int f)
{
	release();
	if (!st)
		return false;
	flags = f;
	graph = make_graph(st, 0, strlen(st), Graph());
	return !graph.empty();
}

int	Sre::match (const char* st)
{
	int end = 0;
	if (!st)
		return 0;
	end = match_graph(st, 0, graph.pfirst);
	if (end > 0) {
		begins[0] = 0;
		ends[0] = end;
	}
	return end;
}

const char*	Sre::search (const char* st)
{
	if (!st)
		return NULL;
	for (int i=0; st[i]; i++) {
		int end = match_graph(st, i, graph.pfirst);
		if (end > i) {
			begins[0] = i;
			ends[0] = end;
			return &st[end];
		}
	}
	return NULL;
}

string Sre::group (const char* st, int ord)
{
	if (ord >= MAX_PARENTHESE_CNT)
		return string();
	return string(st, begins[ord], ends[ord]-begins[ord]);
}

int Sre::get_u8char (const char *st, char *u8c)
{
	size_t u32;
	int clen = StrKit::u8info(st, u32);
	if (clen >= MAX_U8C_LENGTH)
		return 0;
	for (int i=0; i<clen; i+=1)
		u8c[i] = st[i];
	u8c[clen] = 0;
	return clen;
}

Token Sre::get_token (const char *exp, int &pos, char *u8c) 
{
	int clen = get_u8char(exp+pos, u8c);
	pos += clen;

	if (clen==1 && u8c[0]=='\\') {
		clen = get_u8char(exp+pos, u8c);
		pos += clen;
		if (clen==1 && u8c[0]=='a') return ALPHABET;
		if (clen==1 && u8c[0]=='c') return CHINESE;
		if (clen==1 && u8c[0]=='d') return NUMBER;
		if (clen==1 && u8c[0]=='s') return SPACE;
		if (clen==1 && u8c[0]=='A') return NOT_ALPHABET;
		if (clen==1 && u8c[0]=='C') return NOT_CHINESE;
		if (clen==1 && u8c[0]=='D') return NOT_NUMBER;
		if (clen==1 && u8c[0]=='S') return NOT_SPACE;
		if (clen==1 && u8c[0]=='f') {u8c[0] = '\f'; return COMMON;}
		if (clen==1 && u8c[0]=='n') {u8c[0] = '\n'; return COMMON;}
		if (clen==1 && u8c[0]=='r') {u8c[0] = '\r'; return COMMON;}
		if (clen==1 && u8c[0]=='t') {u8c[0] = '\t'; return COMMON;}
		if (clen==1 && u8c[0]=='v') {u8c[0] = '\v'; return COMMON;}
		return COMMON;
	}
	if (clen==0)  return END_REGEXP;
	if (clen==1 && u8c[0]=='(') {parenthese_ord+=1; return LEFT_PARENTHESE;}
	if (clen==1 && u8c[0]==')') return RIGHT_PARENTHESE;
	if (clen==1 && u8c[0]=='|') return BACKTRACE;
	if (clen==1 && u8c[0]=='.') return DOT;
	if (clen==1 && u8c[0]=='^' && pos == 1) return POSI_BEGIN;
	if (clen==1 && u8c[0]=='$' && !exp[pos]) return POSI_END;
	if (clen==1 && u8c[0]=='*') {
		if (exp[pos] == '?') {pos += 1; return REPEAT_ZERO_MORE;}
		else return REPEAT_MORE_ZERO;
	}
	if (clen==1 && u8c[0]=='+') {
		if (exp[pos] == '?') {pos += 1; return REPEAT_ONCE_MORE;}
		else return REPEAT_MORE_ONCE;
	}
	if (clen==1 && u8c[0]=='?') {
		if (exp[pos] == '?') {pos += 1; return REPEAT_ZERO_ONCE;}
		else return REPEAT_ONCE_ZERO;
	}
	return COMMON;
}

Node* Sre::alloc_node (int token, char *u8c, int ord)
{
	Node *p = new Node(token, u8c, ord);
	if (p) collector.push_back(p);
	return p;
}

Node* Sre::alloc_node (const Node &n)
{
	Node *p = new Node(n);
	if (p) collector.push_back(p);
	return p;
}

Graph Sre::make_graph (const char *exp, int s, int e, Graph prev)
{
	Graph grh;
	Node *pn=NULL, *pn2=NULL;
	int i, j, k, tok;
	char u8c[MAX_U8C_LENGTH];

	if (s >= e) return prev;
	tok = get_token(exp, s, u8c);
	switch (tok) {
	case POSI_BEGIN:
	case POSI_END:
	case COMMON:
	case DOT:
	case ALPHABET:
	case CHINESE:
	case NUMBER:
	case SPACE:
	case NOT_ALPHABET:
	case NOT_CHINESE:
	case NOT_NUMBER:
	case NOT_SPACE:
		if ( !(pn=alloc_node(tok, u8c)))
			break;
		if (prev.plast && !prev.plast->add_next(pn))
			break;
		grh = make_graph(exp, s, e, Graph(pn, pn));
		if (!grh.empty())
			return Graph(pn, grh.plast);
		break;
	
	case LEFT_PARENTHESE:
		if ( !(pn=alloc_node(tok, u8c, parenthese_ord)))
			break;
		if ( !(pn2=alloc_node(RIGHT_PARENTHESE, ")", parenthese_ord)))
			break;
		i = s;
		j = 1;
		k = parenthese_ord;
		while ((tok=get_token(exp, s, u8c)) != END_REGEXP) {
			if (tok == LEFT_PARENTHESE) j++;
			if (tok == RIGHT_PARENTHESE) j--;
			if ((tok==BACKTRACE && j==1) || j==0) {
				parenthese_ord = k;
				grh = make_graph(exp, i, s-1, Graph());
				if (grh.empty()) break;
				if (!pn->add_next(grh.pfirst)) break;
				if (!grh.plast->add_next(pn2)) break;
				k = parenthese_ord;
				i = s;
				if (j == 0) break;
			}
		}
		if (tok==END_REGEXP || grh.empty())
			break;
		if (prev.plast && !prev.plast->add_next(pn))
			break;
		grh = make_graph(exp, s, e, Graph(pn, pn2));
		if (!grh.empty())
			return Graph(pn, grh.plast);
		break;
	
	case REPEAT_ZERO_ONCE:
	case REPEAT_ONCE_ZERO:
		if (prev.empty())
			return make_graph(exp, s, e, prev);
		if ( !(pn=alloc_node(*(prev.pfirst))))
			break;
		if ( !(pn2=alloc_node(EMPTY)))
			break;
			
		*(prev.pfirst) = Node(tok, "?");
		if (prev.single()) {
			if (!pn->add_next(pn2))	break;
		}
		else {
			if (!prev.plast->add_next(pn2))	break;
		}
		
		if (tok == REPEAT_ZERO_ONCE) 
		{
			if (!prev.pfirst->add_next(pn2) || !prev.pfirst->add_next(pn))
				break;
		} else {
			if (!prev.pfirst->add_next(pn) || !prev.pfirst->add_next(pn2))
				break;
		}
		grh = make_graph(exp, s, e, Graph(prev.pfirst, pn2));
		if (!grh.empty())
			return Graph(prev.pfirst, grh.plast);
		break;

	case REPEAT_ZERO_MORE:
	case REPEAT_MORE_ZERO:
		if (prev.empty())
			return make_graph(exp, s, e, prev);
		
		if ( !(pn=alloc_node(*(prev.pfirst))))
			break;
		if ( !(pn2=alloc_node(EMPTY)))
			break;

		*(prev.pfirst) = Node(tok, "*");
		if (prev.single()) {
			if (!pn->add_next(prev.pfirst))
				break;
		}
		else {
			if (!prev.plast->add_next(prev.pfirst))
				break;
		}

		if (tok == REPEAT_ZERO_MORE) 
		{
			if (!prev.pfirst->add_next(pn2) || !prev.pfirst->add_next(pn))
				break;
		} else {
			if (!prev.pfirst->add_next(pn) || !prev.pfirst->add_next(pn2))
				break;
		}
		grh = make_graph(exp, s, e, Graph(prev.pfirst, pn2));
		if (!grh.empty())
			return Graph(prev.pfirst, grh.plast);
		break;

	case REPEAT_ONCE_MORE:
	case REPEAT_MORE_ONCE:
		if (prev.empty())
			return make_graph(exp, s, e, prev);

		if ( !(pn=alloc_node(tok, "+")))
			break;
		if ( !(pn2=alloc_node(EMPTY)))
			break;

		if (!prev.plast->add_next(pn))
			break;
		
		if (tok == REPEAT_ONCE_MORE) 
		{
			if (!pn->add_next(pn2) || !pn->add_next(prev.pfirst))
				break;
		} else {
			if (!pn->add_next(prev.pfirst) || !pn->add_next(pn2))
				break;
		}
		grh = make_graph(exp, s, e, Graph(prev.pfirst, pn2));
		if (!grh.empty())
			return Graph(prev.pfirst, grh.plast);
		break;
	default:
		break;
	}
	return Graph();
}

int Sre::match_graph (const char* st, int posi, Node* pnode)
{
	int clen;
	size_t u32;

	if (pnode == NULL)
		return posi;
	
	switch (pnode->token) 
	{
	case COMMON:
		clen = strlen(pnode->u8c);
		for (int i=0; i<clen; i+=1) {
			bool f = flags&IGNORECASE;
			char c1=st[posi+i], c2=pnode->u8c[i];
			if ((!f && c1!=c2) || (f && (tolower(c1)!=tolower(c2))))
				goto match_failed;
		}
		posi += clen;
		break;
	case POSI_BEGIN:
		if (posi != 0) goto match_failed;
		break;
	case POSI_END:
		if (st[posi]) goto match_failed;
		return posi;
	case LEFT_PARENTHESE:
		if (pnode->ord < MAX_PARENTHESE_CNT)
			begins[pnode->ord] = posi;
		break;
	case RIGHT_PARENTHESE:
		if (pnode->ord < MAX_PARENTHESE_CNT)
			ends[pnode->ord] = posi;
		break;
	case DOT:
		clen = StrKit::u8info(st+posi, u32);
		if (clen==0) goto match_failed;
		if (!(flags&DOTALL) && st[posi]=='\n') goto match_failed;
		posi += clen;
		break;
	case ALPHABET:
		if (!isalpha(st[posi])) goto match_failed;
		posi += 1;
		break;
	case CHINESE:
		clen = StrKit::u8info(st+posi, u32);
		if (u32<0x4E00 || u32>0x9FA5) goto match_failed;
		posi += clen;
		break;
	case NUMBER:
		if (!isdigit(st[posi])) goto match_failed;
		posi += 1;
		break;
	case SPACE:
		if (!isspace(st[posi])) goto match_failed;
		posi += 1;
		break;
	case NOT_ALPHABET:
		//if (!st[posi]) goto match_failed;
		if (isalpha(st[posi])) goto match_failed;
		posi += 1;
		break;
	case NOT_CHINESE:
		clen = StrKit::u8info(st+posi, u32);
		//if (clen==0) goto match_failed;
		if (u32>=0x4E00 && u32<=0x9FA5) goto match_failed;
		posi += clen;
		break;
	case NOT_NUMBER:
		//if (!st[posi]) goto match_failed;
		if (isdigit(st[posi])) goto match_failed;
		posi += 1;
		break;
	case NOT_SPACE:
		//if (!st[posi]) goto match_failed;
		if (isspace(st[posi])) goto match_failed;
		posi += 1;
		break;
	default:
		break;
	}
	if (pnode->nnexts == 0)
		return posi;
	for (int i=0; i<pnode->nnexts; i++) {
		int mp = match_graph(st, posi, pnode->pnexts[i]);
		if (mp > 0)	return mp;
	}
match_failed:
	return 0;
}


} // namespace
