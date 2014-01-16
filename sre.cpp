
#include "utils.h"
#include "sre.h"

namespace ucltp {

void SreImpl::release()
{
  for (int i=0; i<_collector.size(); i+=1)
    delete _collector[i];
  _collector.clear();
}

bool SreImpl::compile(const char* exp)
{
  release();
  if (!exp)
    return false;
  _graph = make_graph(exp, 0, strlen(exp), graph_t());
  //_graph._first->show();
  return !_graph.empty();
}

int SreImpl::match(const vector<char_t>& chars, int start)
{
  int end = match_graph(chars, start, _graph._first);
  return end - start;
}

int SreImpl::match_graph(const vector<char_t>& chars, int posi, node_t* pnode)
{
  //printf("match, char[%d]=%4x, %x, (%2d, %4x)\n", posi, chars[posi]._code, (uint64)pnode, pnode->_token, pnode->_code);
  switch (pnode->_token) 
  {
  case COMMON:
    if (posi >= chars.size() || chars[posi]._code != pnode->_code)
      goto match_failed;
    posi += 1;
    break;
  case POSI_BEGIN:
    if (posi != 0)
      goto match_failed;
    break;
  case POSI_END:
    if (posi != chars.size())
      goto match_failed;
    return posi;
  case LEFT_PARENTHESE:
    break;
  case RIGHT_PARENTHESE:
    break;
  case DOT:
    if (posi >= chars.size())
      goto match_failed;
    posi += 1;
    break;
  case ALPHABET:
    if (posi >= chars.size() || !isalpha(chars[posi]._code))
      goto match_failed;
    posi += 1;
    break;
  case NOT_ALPHABET:
    if (posi >= chars.size() || isalpha(chars[posi]._code))
      goto match_failed;
    posi += 1;
    break;
  case CHINESE:
    if (posi >= chars.size() || !ischn(chars[posi]._code))
      goto match_failed;
    posi += 1;
    break;
  case NOT_CHINESE:
    if (posi >= chars.size() || ischn(chars[posi]._code))
      goto match_failed;
    posi += 1;
    break;
  case DIGIT:
    if (posi >= chars.size() || !isdigit(chars[posi]._code))
      goto match_failed;
    posi += 1;
    break;
  case NOT_DIGIT:
    if (posi >= chars.size() || isdigit(chars[posi]._code))
      goto match_failed;
    posi += 1;
    break;
  case NUMBER:
    if (posi >= chars.size() || !isnumber(chars[posi]._code))
      goto match_failed;
    posi += 1;
    break;
  case NOT_NUMBER:
    if (posi >= chars.size() || isnumber(chars[posi]._code))
      goto match_failed;
    posi += 1;
    break;
  case SPACE:
    if (posi >= chars.size() || !isspace(chars[posi]._code))
      goto match_failed;
    posi += 1;
    break;
  case NOT_SPACE:
    if (posi >= chars.size() || isspace(chars[posi]._code))
      goto match_failed;
    posi += 1;
    break;
  default:
    break;
  }
  if (pnode->_nexts.size() == 0)
    return posi;
  for (int i=0; i<pnode->_nexts.size(); i+=1) {
    int end = match_graph(chars, posi, pnode->_nexts[i]);
    if (end > 0)
      return end;
  }
match_failed:
  return 0;
}

token_t SreImpl::get_token(const char *exp, int &pos, uint32 &code) 
{
  uint32 clen;

  get_utf8_char(exp+pos, clen, code);
  pos += clen;

  if (clen==1 && char(code)=='\\') {
    get_utf8_char(exp+pos, clen, code);
    pos += clen;
    if (clen==1 && char(code)=='a') return ALPHABET;
    if (clen==1 && char(code)=='c') return CHINESE;
    if (clen==1 && char(code)=='d') return DIGIT;
    if (clen==1 && char(code)=='n') return NUMBER;
    if (clen==1 && char(code)=='s') return SPACE;
    if (clen==1 && char(code)=='A') return NOT_ALPHABET;
    if (clen==1 && char(code)=='C') return NOT_CHINESE;
    if (clen==1 && char(code)=='D') return NOT_DIGIT;
    if (clen==1 && char(code)=='N') return NOT_NUMBER;
    if (clen==1 && char(code)=='S') return NOT_SPACE;
    if (clen==1 && char(code)=='f') {code = uint32('\f'); return COMMON;}
    if (clen==1 && char(code)=='n') {code = uint32('\n'); return COMMON;}
    if (clen==1 && char(code)=='r') {code = uint32('\r'); return COMMON;}
    if (clen==1 && char(code)=='t') {code = uint32('\t'); return COMMON;}
    if (clen==1 && char(code)=='v') {code = uint32('\v'); return COMMON;}
    return COMMON;
  }
  if (clen==0)  return END_REGEXP;
  if (clen==1 && char(code)=='(') return LEFT_PARENTHESE;
  if (clen==1 && char(code)==')') return RIGHT_PARENTHESE;
  if (clen==1 && char(code)=='|') return BACKTRACE;
  if (clen==1 && char(code)=='.') return DOT;
  if (clen==1 && char(code)=='^' && pos == 1) return POSI_BEGIN;
  if (clen==1 && char(code)=='$' && !exp[pos]) return POSI_END;
  if (clen==1 && char(code)=='*') {
    if (exp[pos] == '?') {pos += 1; return REPEAT_ZERO_MORE;}
    else return REPEAT_MORE_ZERO;
  }
  if (clen==1 && char(code)=='+') {
    if (exp[pos] == '?') {pos += 1; return REPEAT_ONCE_MORE;}
    else return REPEAT_MORE_ONCE;
  }
  if (clen==1 && char(code)=='?') {
    if (exp[pos] == '?') {pos += 1; return REPEAT_ZERO_ONCE;}
    else return REPEAT_ONCE_ZERO;
  }
  return COMMON;
}

node_t* SreImpl::alloc_node(int token, uint32 code)
{
  node_t *p = new node_t(token, code);
  if (p) _collector.push_back(p);
  return p;
}

node_t* SreImpl::alloc_node(const node_t &n)
{
  node_t *p = new node_t(n);
  if (p) _collector.push_back(p);
  return p;
}

graph_t SreImpl::make_graph(const char *exp, int s, int e, graph_t prev)
{
  graph_t grh;
  node_t *pn=NULL, *pn2=NULL;
  int i, j, k, tok;
  uint32 code;

  if (s >= e) return prev;
  int olds = s;
  tok = get_token(exp, s, code);
  //printf("exp=%s, tok=%2d, code=%4x\n", exp+olds, tok, code);
  switch (tok) {
  case POSI_BEGIN:
  case POSI_END:
  case COMMON:
  case DOT:
  case ALPHABET:
  case CHINESE:
  case DIGIT:
  case NUMBER:
  case SPACE:
  case NOT_ALPHABET:
  case NOT_CHINESE:
  case NOT_DIGIT:
  case NOT_NUMBER:
  case NOT_SPACE:
    pn = alloc_node(tok, code);
    if (!pn)
      break;
    if (prev._last)
      prev._last->add_next(pn);
    grh = make_graph(exp, s, e, graph_t(pn, pn));
    if (!grh.empty())
      return graph_t(pn, grh._last);
    break;
  
  case LEFT_PARENTHESE:
    pn = alloc_node(tok, code);
    pn2 = alloc_node(RIGHT_PARENTHESE, uint32(')'));
    if (!pn || !pn2)
      break;
    i = s;
    j = 1;
    while ((tok = get_token(exp, s, code)) != END_REGEXP) {
      if (tok == LEFT_PARENTHESE) j++;
      if (tok == RIGHT_PARENTHESE) j--;
      if ((tok==BACKTRACE && j==1) || j==0) {
        grh = make_graph(exp, i, s-1, graph_t());
        if (grh.empty()) break;
        pn->add_next(grh._first);
        grh._last->add_next(pn2);
        i = s;
        if (j == 0) break;
      }
    }
    if (tok==END_REGEXP || grh.empty())
      break;
    if (prev._last)
      prev._last->add_next(pn);
    grh = make_graph(exp, s, e, graph_t(pn, pn2));
    if (!grh.empty())
      return graph_t(pn, grh._last);
    break;
  
  case REPEAT_ZERO_ONCE:
  case REPEAT_ONCE_ZERO:
    if (prev.empty())
      return make_graph(exp, s, e, prev);
    pn = alloc_node(*(prev._first));
    pn2 = alloc_node(EMPTY);
    if (!pn || !pn2)
      break;
      
    *(prev._first) = node_t(tok, uint32('?'));
    if (prev.single())
      pn->add_next(pn2);
    else
      prev._last->add_next(pn2);
    
    if (tok == REPEAT_ZERO_ONCE) 
    {
      prev._first->add_next(pn2);
      prev._first->add_next(pn);
    } else {
      prev._first->add_next(pn);
      prev._first->add_next(pn2);
    }
    grh = make_graph(exp, s, e, graph_t(prev._first, pn2));
    if (!grh.empty())
      return graph_t(prev._first, grh._last);
    break;

  case REPEAT_ZERO_MORE:
  case REPEAT_MORE_ZERO:
    if (prev.empty())
      return make_graph(exp, s, e, prev);
    pn = alloc_node(*(prev._first));
    pn2 = alloc_node(EMPTY);
    if (!pn || !pn2)
      break;

    *(prev._first) = node_t(tok, uint32('*'));
    if (prev.single())
      pn->add_next(prev._first);
    else
      prev._last->add_next(prev._first);

    if (tok == REPEAT_ZERO_MORE) 
    {
      prev._first->add_next(pn2);
      prev._first->add_next(pn);
    } else {
      prev._first->add_next(pn);
      prev._first->add_next(pn2);
    }
    grh = make_graph(exp, s, e, graph_t(prev._first, pn2));
    if (!grh.empty())
      return graph_t(prev._first, grh._last);
    break;

  case REPEAT_ONCE_MORE:
  case REPEAT_MORE_ONCE:
    if (prev.empty())
      return make_graph(exp, s, e, prev);
    pn = alloc_node(tok, uint32('+'));
    pn2 = alloc_node(EMPTY);
    
    prev._last->add_next(pn);
    if (tok == REPEAT_ONCE_MORE) 
    {
      pn->add_next(pn2);
      pn->add_next(prev._first);
    } else {
      pn->add_next(prev._first);
      pn->add_next(pn2);
    }
    grh = make_graph(exp, s, e, graph_t(prev._first, pn2));
    if (!grh.empty())
      return graph_t(prev._first, grh._last);
    break;
  default:
    break;
  }
  return graph_t();
}

int Sre::build(const char* fre)
{
  vector<string> lines;
  vector<string> parts;

  _objects.clear();

  read_lines(fre, lines);
  for (int i=0; i<lines.size(); i+=1) {
    split(lines[i], parts);
    if (parts.size() == 2) {
      int value = atoi(parts[1].c_str());
      SreImpl* obj = new SreImpl;
      _objects.push_back(std::make_pair(obj, value));
      _objects.back().first->compile(parts[0].c_str());
    }
  }
}

match_result_t Sre::match(const vector<char_t>& chars, int start)
{
  match_result_t r;
  for (int i=0; i<_objects.size(); i+=1) {
    int len = _objects[i].first->match(chars, start);
    if (len > r.len) {
      r.len = len;
      r.value = _objects[i].second;
    }
  }
  return r;
}

} // namespace
