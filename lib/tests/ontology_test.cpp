/**
 * Tests related to the ontology.h class
 * */

#include "catch.hpp"


#include <memory>
#include <iostream>

#include "catch.hpp"
#include "../base.pb.h"
#include "../phenopackets.pb.h"
#include "../interpretation.pb.h"
#include "../phenotools.h"
#include "../ontology.h"
#include "../jsonobo.h"
#include <google/protobuf/message.h>
#include <google/protobuf/util/json_util.h>

using std::cout;
using std::cerr;



TEST_CASE("Test exists path algorithm","[exists_path]") {
  string hp_json_path = "../testdata/hp.small.json";
  JsonOboParser parser {hp_json_path};
  std::unique_ptr<Ontology>  ontology = parser.get_ontology();
  // term 1 is the root
  TermId t1 = TermId::from_string("HP:0000001");
  // term 2 is the child of term 1
  TermId t2 = TermId::from_string("HP:0000002");
  // term 3 is the grand child of term 1 and child of term 2
  TermId t3 = TermId::from_string("HP:0000003");
  // term 4 is the  child of term 1 but not child of term 2
  TermId t4 = TermId::from_string("HP:0000004");
  // term 5 is the grand child of term 1 but not child of term 2
  TermId t5 = TermId::from_string("HP:0000005");
  bool b = ontology->exists_path(t2,t1);
  REQUIRE(b);
  b = ontology->exists_path(t3,t1);
  REQUIRE(b);
  REQUIRE(ontology->exists_path(t1,t3, EdgeType::IS_A_INVERSE));
  b = ontology->exists_path(t5,t3);
  REQUIRE(!b);
  b = ontology->exists_path(t5,t4);
  REQUIRE(b);
  b = ontology->exists_path(t5,t1);
  REQUIRE(b);
  REQUIRE_FALSE(ontology->exists_path(t5,t2));
}

TEST_CASE("Test exists path algorithm with specific edge type","[exists_path_edgetype]") {
  string hp_json_path = "../testdata/hp.small.json";
  JsonOboParser parser {hp_json_path};
  std::unique_ptr<Ontology>  ontology = parser.get_ontology();
  // term 1 is the root
  TermId t1 = TermId::from_string("HP:0000001");
  // term 2 is the child of term 1
  TermId t2 = TermId::from_string("HP:0000002");
  // term 3 is the grand child of term 1 and child of term 2
  TermId t3 = TermId::from_string("HP:0000003");
  // term 4 is the  child of term 1 but not child of term 2
  TermId t4 = TermId::from_string("HP:0000004");
  // term 5 is the grand child of term 1 but not child of term 2
  TermId t5 = TermId::from_string("HP:0000005");
  bool b = ontology->exists_path(t2,t1, EdgeType::IS_A);
  REQUIRE(b);
  b = ontology->exists_path(t1, t2, EdgeType::IS_A);
  REQUIRE_FALSE(b);
  b = ontology->exists_path(t1, t2, EdgeType::IS_A_INVERSE);
  REQUIRE(b);
  b = ontology->exists_path(t3, t1, EdgeType::IS_A);
  REQUIRE(b);
  b = ontology->exists_path(t1, t3, EdgeType::IS_A_INVERSE);
  REQUIRE(b);
  // t3 is grand child of t1
  REQUIRE(ontology->exists_path(t3, t1, EdgeType::IS_A));
  // t5 is grandchild of t1 and child of t4 but not descendant of t3
  REQUIRE_FALSE(ontology->exists_path(t5, t3, EdgeType::IS_A));
  // t5 is child of t4
  REQUIRE(ontology->exists_path(t5, t4, EdgeType::IS_A));
  REQUIRE_FALSE(ontology->exists_path(t4, t5));
  REQUIRE_FALSE(ontology->exists_path(t5, t4, EdgeType::IS_A_INVERSE));
  REQUIRE(ontology->exists_path(t5, t1));
  // t5 and t2 are in different subhierarchies
  REQUIRE_FALSE(ontology->exists_path(t5, t2));
}
