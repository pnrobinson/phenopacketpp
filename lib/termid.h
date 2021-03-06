#ifndef TERMID_H
#define TERMID_H


#include <string>
using std::string;

/**
 * Class to represent the identifier of an ontology term, e.g., HP:0001234.
 */
class TermId {
 private:
  string value_;
  std::size_t separator_pos_;
  TermId(const string &s, std::size_t pos);

 public:
  TermId(const TermId  &tid);
  TermId(TermId &&tid);
  TermId &operator=(const TermId &tid);
  TermId &operator=(TermId &&tid);
  bool operator<(const TermId& rhs) const;
  ~TermId(){}
  static TermId from_string(const string &s);
  static TermId from_url(const string &s);
  string get_value() const { return value_; }
  string get_prefix() const { return value_.substr(0,separator_pos_); }
  string get_id() const { return value_.substr(separator_pos_+1); }
  friend bool operator==(const TermId& lhs, const TermId& tid);
  friend bool operator!=(const TermId& lhs, const TermId& tid);
  friend std::ostream& operator<<(std::ostream& ost, const TermId& tid);
};
bool operator==(const TermId& lhs, const TermId& tid);
bool operator!=(const TermId& lhs, const TermId& tid);
std::ostream& operator<<(std::ostream& ost, const TermId& tid);

// static instance
extern const TermId EMPTY_TERMID;

#endif
