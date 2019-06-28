#ifndef ONTOLOGY_H
#define ONTOLOGY_H

#include <string>
#include <vector>
#include <rapidjson/document.h>
#include "jsonparse_exception.h"

using std::string;
using std::vector;



class TermId {
private:
    string value_;
    std::size_t separator_pos_;
    TermId(const string &s,std::size_t pos);
    
public:
   
    TermId(const TermId  &tid);
    TermId(TermId &&tid);
    TermId &operator=(const TermId &tid);
    ~TermId(){}
    static TermId of(const string &s);
    static TermId of(const rapidjson::Value &val);
    string get_value() const { return value_; }
    string get_prefix() const { return value_.substr(0,separator_pos_); }
    string get_id() const { return value_.substr(separator_pos_+1); }
    friend std::ostream& operator<<(std::ostream& ost, const TermId& tid);
};
std::ostream& operator<<(std::ostream& ost, const TermId& tid);

class Xref {
private:
    TermId term_id_;
    Xref(const TermId &tid): term_id_(tid){}
   
public:
    Xref(const Xref &txr);
    Xref(Xref &&txr) = default;
    Xref &operator=(const Xref &txr);
    static Xref of(const rapidjson::Value &val);
    static Xref fromCurieString(const rapidjson::Value &val);
    friend std::ostream& operator<<(std::ostream& ost, const Xref& txref);
};
std::ostream& operator<<(std::ostream& ost, const Xref& txref);



class Term {
private:
  string id_;
  string label_;
  string definition_;
  vector<Xref> definition_xref_list_;
  vector<Xref> term_xref_list_;
  bool is_obsolete = false;

public:
  Term(const string &id, const string &label);
  void add_definition(const string &def);
  void add_definition_xref(const Xref &txref);
  void add_term_xref(const Xref &txref) { term_xref_list_.push_back(txref); }

  friend std::ostream& operator<<(std::ostream& ost, const Term& term);
};
std::ostream& operator<<(std::ostream& ost, const Term& term);

#endif
