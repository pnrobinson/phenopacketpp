#ifndef ONTOLOGY_H
#define ONTOLOGY_H

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <optional>
#include <rapidjson/document.h>
#include "termid.h"
#include "edge.h"
#include "property.h"
#include "jsonparse_exception.h"

using std::string;
using std::vector;
using std::map;


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
    TermId get_termid() const { return term_id_; }
    friend std::ostream& operator<<(std::ostream& ost, const Xref& txref);
};
std::ostream& operator<<(std::ostream& ost, const Xref& txref);

class Term {
private:
  TermId id_;
  string label_;
  string definition_;
  vector<Xref> definition_xref_list_;
  vector<Xref> term_xref_list_;
  vector<TermId> alternative_id_list_;
  vector<PropertyValue> property_values_;
  bool is_obsolete_ = false;

public:
  Term(const TermId &id, const string &label);
  //static Term of(const rapidjson::Value &val);
  void add_definition(const string &def);
  void add_definition_xref(const Xref &txref);
  void add_term_xref(const Xref &txref) { term_xref_list_.push_back(txref); }
  void add_property_value(const PropertyValue &pv);

  TermId get_term_id() const { return id_; }
  string get_label() const { return label_; }
  string get_definition() const { return definition_; }
  vector<Xref> get_definition_xref_list() const { return definition_xref_list_;}
  vector<Xref> get_term_xref_list() const {return term_xref_list_;}
  bool has_alternative_ids() const { return ! alternative_id_list_.empty(); }
  vector<TermId> get_alternative_ids() const { return alternative_id_list_; }
  vector<PropertyValue> get_property_values() const { return property_values_; }
  vector<TermId> get_isa_parents(const TermId &child) const;
  bool obsolete() const { return is_obsolete_; }
  friend std::ostream& operator<<(std::ostream& ost, const Term& term);
};
std::ostream& operator<<(std::ostream& ost, const Term& term);


class Ontology {
private:
  string id_;
  vector<PropertyValue> property_values_;
  vector<Property> property_list_;
  map<TermId, std::shared_ptr<Term> > term_map_;
  /** Current primary TermId's. */
  vector<TermId> current_term_ids_;
  /** obsoleted and alt ids. */
  vector<TermId> obsolete_term_ids_;
  /** Key: a TermId object. Value: Correspodning index in current_term_ids_. */
  map<TermId, int> termid_to_index_;
  /**  offset_e stores offsets into e_to that indicate where the adjacency lists begin.
  The list for an arbitrary vertex begins at e_to[offset_e[v]] and ends at
  e_to[offset_e[v+1]]-1. */
  vector<int> offset_e_;
  /** CSR (Compressed Storage Format) Adjacency list. */
  vector<int> e_to_;


public:
  Ontology() = default;
  Ontology(const Ontology &other);
  Ontology(Ontology &other);
  Ontology& operator=(const Ontology &other);
  Ontology& operator=(Ontology &&other);
  ~Ontology(){}
  void set_id(const string &id) { id_ = id; }
  void add_property_value(const PropertyValue &propval);
  void add_property(const Property & prop);
  void add_all_terms(const vector<Term> &terms);
  void add_all_edges(vector<Edge> &edges);
  int current_term_count() const { return current_term_ids_.size(); }
  int total_term_id_count() const { return term_map_.size(); }
  int edge_count() const { return e_to_.size(); }
  int property_count() const { return property_list_.size(); }
  std::optional<Term> get_term(const TermId &tid) const;
  vector<TermId> get_isa_parents(const TermId &child) const;
  Ontology(vector<Term> terms,vector<Edge> edges,string id, vector<PropertyValue> properties);
  vector<TermId> get_current_term_ids() const { return current_term_ids_; }
  friend std::ostream& operator<<(std::ostream& ost, const Ontology& ontology);
};
std::ostream& operator<<(std::ostream& ost, const Ontology& ontology);

#endif
