
#include "jsonobo.h"


#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/prettywriter.h>

#include <iostream>
#include <fstream>
#include <sstream>

using std::cout;
using std::cerr;

/**
 * A convenience function for debugging only!
 */
void
printJ(const rapidjson::Value &json)
{
  using namespace rapidjson;
  StringBuffer sb;
  PrettyWriter<StringBuffer> writer(sb);
  json.Accept(writer);
  auto str = sb.GetString();
  std::cout << str << "\n";
}

/**
 * generate a printable String from an arbitrary RAPIDJSON value object.
 * Used mainly to create error messages (if any)
 * @param json A rapid json object
 * @return a String representation of the JSON.
 */
string get_json_string(const rapidjson::Value &json)
{
  using namespace rapidjson;
  StringBuffer sb;
  PrettyWriter<StringBuffer> writer(sb);
  json.Accept(writer);
  return sb.GetString();
}




bool is_class(const rapidjson::Value &val){
  if (! val.IsObject()) {
    return false;
  }
  if (! val.HasMember("type")) {
    return false;
  } else if (! strcmp(val["type"].GetString(), "CLASS")) {
    return true;
  }
  return false;
}

bool is_property(const rapidjson::Value &val){
  if (! val.IsObject()) {
    return false;
  }
  if (! val.HasMember("type")) {
    return false;
  } else if (! strcmp(val["type"].GetString(), "PROPERTY")) {
    return true;
  }
  return false;
}

void
JsonOboParser::process_metadata(const rapidjson::Value &val){
  if (! val.IsObject()) {
    throw JsonParseException("Attempt to add malformed meta (not JSON object).");
  }
  auto itr = val.FindMember("basicPropertyValues");
  if (itr != val.MemberEnd()) {
    const rapidjson::Value &propertyVals = itr->value;
    if (! propertyVals.IsArray()) {
      throw JsonParseException("Ontology property values not array");
    }
    for (auto elem = propertyVals.Begin(); elem != propertyVals.End(); elem++) {
      PredicateValue predval = json_to_property_value(*elem);
      property_value_list_.push_back(predval);
      ontology_.add_predicate_value(predval);
    }
  }
}

void
JsonOboParser::process_nodes(const rapidjson::Value& nodes)
{
  if (! nodes.IsArray()) {
    throw JsonParseException("rapidjson nodes object is not array");
  }
  string myError;
  for (auto& v : nodes.GetArray()) {
    if (is_class(v)) {
      try {
	       Term term = json_to_term(v);
	       term_list_.push_back(term);
      } catch (const JsonParseException& e) {
	       string json_stanza = get_json_string(v);
         string message = e.what();
         std::stringstream sstr;
         sstr << "[ERROR] " << e.what() << "; generated by ("
              << get_json_string(v);
         error_list_.push_back(sstr.str());
        }
    } else if (is_property(v)){
      // This is a PROPERTY element (these are mixed with the CLASS elements
      // in the JSON file), such as "UK spelling", "abbreviation" and so on.
      // These elements introduce properties that can be used elsewhere, for
      // instance, some synonyms have the property "UK spelling"
      try{
	       Property prop = json_to_property(v);
         cerr << "Making property and exiting, jsonobo118: \"" << prop <<"\"\n";
         exit(1);
	       ontology_.add_property(prop);
      } catch (const JsonParseException& e) {
         std::stringstream sstr;
         sstr << "[ERROR] Could not create Node" << e.what() << "; generated by ("
              << get_json_string(v);
         error_list_.push_back(sstr.str());
      }
    } else {
      std::cerr << "[ERROR] Neither node nor property: ";
      printJ(v);
      exit(1);
    }
  }
}

void
JsonOboParser::process_edges(const rapidjson::Value& edges)
{
  if (! edges.IsArray()) {
    throw JsonParseException("rapidjson edges object is not array");
  }
  for (auto& v : edges.GetArray()) {
    try {
        Edge e = Edge::of(v);
        edge_list_.push_back(e);
    } catch (const JsonParseException& e) {
      std::stringstream sstr;
      sstr << "[ERROR] Could not create Edge: " << e.what() << "; generated by ("
           << get_json_string(v);
      error_list_.push_back(sstr.str());
    }
  }
}

JsonOboParser::JsonOboParser(const string path):
  path_(path) {
  rapidjson::Document d;
  std::cout << "[INFO] Parsing " << path_ << "\n";
  std::ifstream ifs(path_);
  rapidjson::IStreamWrapper isw(ifs);

  d.ParseStream(isw);
  const rapidjson::Value& a = d["graphs"];
  if (! a.IsArray()) {
    throw JsonParseException("Ontology JSON did not contain graphs element array.");
  }
  // the first item in the array is an object with a list of nodes
  if( a.Size() < 1){
    throw JsonParseException("Ontology JSON did not contain array of nodes.");
  }
  const rapidjson::Value& mainObject = a[0];
  if ( ! mainObject.IsObject()) {
    throw JsonParseException("Main object was not a rapidjson object.");
  }
  const rapidjson::Value& nodes = mainObject["nodes"];
  process_nodes(nodes);

  string myError;
  rapidjson::Value::ConstMemberIterator itr = mainObject.FindMember("edges");
  if (itr == mainObject.MemberEnd()){
    throw JsonParseException("Did not find edges element");
  }
  const rapidjson::Value& edges = mainObject["edges"];
  process_edges(edges);


  itr = mainObject.FindMember("id");
  if (itr == mainObject.MemberEnd()){
    throw JsonParseException("Did not find id element");
  } else {
    ontology_id_ = itr->value.GetString();
  }
  itr = mainObject.FindMember("meta");
  if (itr == mainObject.MemberEnd()){
    throw JsonParseException("Did not find meta element");
  } else {
    const rapidjson::Value& meta = mainObject["meta"];
    process_metadata(meta);
  }
  std::cout << "DONE:" <<  std::endl;
  if (error_list_.size()>0) {
    for (string e : error_list_) {
      std::cout << "\t" << e << "\n";
    }
  } else {
    std::cout << "[INFO] No errors encountered\n";
  }
}




void
JsonOboParser::transform_input_data_to_ontology()
{
vector<int> offset_e_;

}

void
JsonOboParser::dump_errors() const
{
  if (error_list_.size() == 0) {
    std::cout <<"[INFO] No errors enounted in JSON parse\n";
    return;
  }
  std::cout << "[ERRORS]:\n";
  for (string e : error_list_) {
    std::cout << e << "\n";
  }
}


Ontology
JsonOboParser::get_ontology()
{
  dump_errors();
  ontology_.set_id(ontology_id_);
  ontology_.add_all_terms(term_list_);
  ontology_.add_all_edges(edge_list_);
  return ontology_;
}



/**
 * construct a PropertyValue from a JSON object
 */
PredicateValue
JsonOboParser::json_to_property_value(const rapidjson::Value &val) {
  if (! val.IsObject()) {
    throw JsonParseException("PropertyValue factory expects object");
  }
  auto p = val.FindMember("pred");
  if (p == val.MemberEnd()) {
    throw JsonParseException("PropertyValue did not contain \'pred\' element");
  }
  // PropertyValue elements may contain elements like this
  // "pred" : "http://purl.org/dc/elements/1.1/creator",
  // In this case, we extract the last subelement (creator)
  string pred = val["pred"].GetString();
  size_t pos = pred.find_last_of('/');
  if (pos != string::npos) {
    pred = pred.substr(pos+1);
  }
  //We keep a list of properties in
  Predicate predicate = PredicateValue::string_to_predicate(pred);

  p = val.FindMember("val");
  if (p == val.MemberEnd()) {
    throw JsonParseException("PropertyValue did not contain \'val\' element");
  }

  string valu = val["val"].GetString();
  PredicateValue pv{predicate,valu};
  return pv;
}


Term
JsonOboParser::json_to_term(const rapidjson::Value &val){
  string id;
  string label;
  if (! val.IsObject()) {
    throw JsonParseException("Attempt to add malformed node (not JSON object)");
  }
  if (! val.HasMember("type")) {
    throw JsonParseException("Attempt to add malformed node (no type information).");
  } else if (strcmp ( val["type"].GetString(),"CLASS") ) {
    string tt =val["type"].GetString();
    throw JsonParseException("Attempt to add malformed node (not a CLASS): " + tt);
  }
  if (! val.HasMember("id")) {
    throw JsonParseException("Attempt to add malformed node (no id).");
  } else {
    id = val["id"].GetString();
  }
  if (! val.HasMember("lbl")) {
    //throw JsonParseException("Malformed node ("+id+"): no label.");
    cerr << "[WARNING] node  ("+id+"): no label.\n";
  } else {
    label = val["lbl"].GetString();
  }
  TermId tid = TermId::of(id);
  Term term{tid,label};
  if (! val.HasMember("meta")) {
    //throw JsonParseException("Malformed node ("+id+"): no Metainformation");
    cerr << "[WARNING] node (" << id << ") has no Metainformation\n";
  } else {
    const rapidjson::Value &meta = val["meta"];
    if (! meta.IsObject()) {
      throw JsonParseException("Malformed node ("+id+"): meta is not JSON object.");
    }
    rapidjson::Value::ConstMemberIterator itr = meta.FindMember("definition");
    if (itr != meta.MemberEnd()) {
      const rapidjson::Value &definition = meta["definition"];
      rapidjson::Value::ConstMemberIterator it = definition.FindMember("val");
      if (it != definition.MemberEnd()) {
	string definition_value = it->value.GetString();
	term.add_definition(definition_value);
      }
      it = definition.FindMember("xrefs");
      if (it != definition.MemberEnd()) {
	const rapidjson::Value& defxrefs = it->value;
	if (! defxrefs.IsArray()) {
	  throw JsonParseException("Malformed node ("+id+"): xref not array");
	}
	for (auto xrefs_itr = defxrefs.Begin();
	     xrefs_itr != defxrefs.End(); ++xrefs_itr) {
	  Xref xr = Xref::fromCurieString(*xrefs_itr); // xrefs in definitions are simply CURIEs.
	  term.add_definition_xref(xr);
        }
      } // done with definition
      itr = meta.FindMember("xrefs");
      if (itr != meta.MemberEnd()) {
	const rapidjson::Value &xrefs = itr->value;
	if (! xrefs.IsArray()) {
	  throw JsonParseException("Malformed node ("+id+"): Term Xrefs not array");
	} else {
	  for (auto elem = xrefs.Begin(); elem != xrefs.End(); elem++) {
	    auto elem_iter = elem->FindMember("val");
	    if (elem_iter != elem->MemberEnd()) {
	      Xref txr = Xref::of(elem_iter->value);
	      term.add_term_xref(txr);
	    }
          }
	}
	itr = meta.FindMember("basicPropertyValues");
	if (itr != meta.MemberEnd()) {
	  const rapidjson::Value &propertyVals = itr->value;
	  if (! propertyVals.IsArray()) {
	    throw JsonParseException("Malformed node ("+id+"): Term property values not array");
	  }
	  for (auto elem = propertyVals.Begin(); elem != propertyVals.End(); elem++) {
	    PredicateValue propval = json_to_property_value(*elem);
	    term.add_predicate_value(propval);
	  }
	}
      }
    }
  }
  return term;
}


Property
JsonOboParser::json_to_property(const rapidjson::Value &val){
  string id;
  string label;
  vector<PredicateValue> propvals;
  if (! val.IsObject()) {
    throw JsonParseException("Attempt to add malformed node (not JSON object)");
  }
  if (! val.HasMember("type")) {
    throw JsonParseException("Attempt to add malformed node (no type information).");
  } else if (strcmp ( val["type"].GetString(),"PROPERTY") ) {
    string tt =val["type"].GetString();
    throw JsonParseException("Attempt to add malformed property node (not a PROPERTY): " + tt);
  }
  if (! val.HasMember("id")) {
    throw JsonParseException("Attempt to add malformed node (no id).");
  } else {
    id = val["id"].GetString();
  }
  if (! val.HasMember("lbl")) {
    throw JsonParseException("Malformed node ("+id+"): no label.");
  } else {
    label = val["lbl"].GetString();
  }
  TermId tid = TermId::of(id);
  if (! val.HasMember("meta")) {
    //throw JsonParseException("Malformed node ("+id+"): no Metainformation");
    cerr << "[WARNING] Property has no ("+id+"): no Metainformation\n";
  } else {
    const rapidjson::Value &meta = val["meta"];
    if (! meta.IsObject()) {
      throw JsonParseException("Malformed node ("+id+"): meta is not JSON object.");
    }
    auto itr = meta.FindMember("basicPropertyValues");
    if (itr != meta.MemberEnd()) {
      const rapidjson::Value &propertyVals = itr->value;
      if (! propertyVals.IsArray()) {
	throw JsonParseException("Malformed node ("+id+"): Term property values not array");
      }
      for (auto elem = propertyVals.Begin(); elem != propertyVals.End(); elem++) {
	PredicateValue propval = json_to_property_value(*elem);
	propvals.push_back(propval);
      }
    }
  }
  Property p{tid,label,propvals};
  return p;
}
