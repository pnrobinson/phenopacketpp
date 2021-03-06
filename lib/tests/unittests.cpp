#define CATCH_CONFIG_MAIN

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

// The arena object is used to allocate certain protobuf objects
google::protobuf::Arena arena;

TEST_CASE("test OntologyClass basics","[ontologyclass]") {
  org::phenopackets::schema::v1::core::OntologyClass ontclz;
  string hpo_id = "HP:0001166";
  string hpo_label = "Arachnodactyly";
  ontclz.set_id(hpo_id);
  ontclz.set_label(hpo_label );
  REQUIRE( ontclz.IsInitialized());
  REQUIRE( ! ontclz.id().empty() );
  REQUIRE(ontclz.id() == hpo_id);
  REQUIRE(ontclz.label() == hpo_label);
}

TEST_CASE("test OntologyClass malformed","[ontologyclass]") {
  org::phenopackets::schema::v1::core::OntologyClass ontclz;
  string hpo_id = "HP0001166"; // invalid id -- no semicolon
  string hpo_label = ""; // invalid label: empty
  ontclz.set_id(hpo_id);
  ontclz.set_label(hpo_label);
  phenotools::OntologyClass myoc(ontclz);
  vector<phenotools::Validation> validation = myoc.validate();
  // we assume here that the id is tested first (v1) and then the label (v2)
  REQUIRE( validation.size() == 2 );
  phenotools::Validation v1 = validation.at(0);
  REQUIRE( v1.is_warning() == true );
  REQUIRE( v1.message() == "OntologyClass id (HP0001166) not a valid CURIE" );
  v1 = validation.at(1);
  REQUIRE( v1.is_warning() == false );
  REQUIRE( v1.is_error() == true );
  REQUIRE( v1.message() == "OntologyClass label not allowed to be empty" );
}

TEST_CASE("test Age with string and empty","[age]") {
  org::phenopackets::schema::v1::core::Age agepb;
  string age40 = "P40Y"; // 40 years old
  agepb.set_age(age40);
  phenotools::Age age(agepb);
  vector<phenotools::Validation> validation = age.validate();
  REQUIRE( validation.empty() == true );
  agepb.clear_age(); // now the Age protobuf class has neither age nor age_class
  // this is an error
  phenotools::Age age2(agepb);
  validation = age2.validate();
  REQUIRE( validation.size() == 1 );
  phenotools::Validation v1 = validation.at(0);
  REQUIRE( v1.is_error() == true );
  REQUIRE( v1.message() == "At least one of age and age_class must be present in Age element" );
}

TEST_CASE("AgeRange","[agerange]") {
  org::phenopackets::schema::v1::core::Age* agepb1 =
    google::protobuf::Arena::Create<org::phenopackets::schema::v1::core::Age>(&arena);
  string age40 = "P40Y"; // 40 years old
  agepb1->set_age(age40);

  org::phenopackets::schema::v1::core::Age* agepb2 =
    google::protobuf::Arena::Create<org::phenopackets::schema::v1::core::Age>(&arena);
  string age50 = "P50Y"; // 40 years old
  agepb2->set_age(age50);

  org::phenopackets::schema::v1::core::AgeRange agerangepb;
  agerangepb.set_allocated_start(agepb1);
  agerangepb.set_allocated_end(agepb2);

  phenotools::AgeRange ar(agerangepb);

  vector<phenotools::Validation> validation = ar.validate();
  REQUIRE(validation.size()==0);

  // Now make one of the elements invalid
  agepb1->set_age("");
  agerangepb.release_start();
  agerangepb.set_allocated_start(agepb1);
  phenotools::AgeRange ar2(agerangepb);
  validation = ar2.validate();
  REQUIRE(validation.size()==1);
  agerangepb.release_start();
  agerangepb.release_end();
}


TEST_CASE("test Individual","[individual]") {
  // All we need to get a valid Individual element is an id
  // if no Sex or Age information is provided, then two
  // warnings are generated.
  org::phenopackets::schema::v1::core::Individual individualpb;
  individualpb.set_id("42");

  phenotools::Individual ind(individualpb);
  vector<phenotools::Validation> validation = ind.validate();
  REQUIRE(validation.size()==2);
  phenotools::Validation v1 = validation.at(0);
  REQUIRE(v1.get_cause() == phenotools::ValidationCause::INDIVIDUAL_LACKS_AGE);
  REQUIRE(v1.is_warning() == true );
  phenotools::Validation v2 = validation.at(1);
  REQUIRE(v2.get_cause() == phenotools::ValidationCause::UNKNOWN_SEX);
  REQUIRE(v2.is_warning() == true );
  // Now let's generate an error by trying to create an individual
  // with an id
  individualpb.clear_id();
  phenotools::Individual ind2(individualpb);
  validation = ind2.validate();
  REQUIRE(validation.size()==3);
  v1 = validation.at(0);
  REQUIRE(v1.get_cause() == phenotools::ValidationCause::INDIVIDUAL_LACKS_ID);
  REQUIRE(v1.is_error() == true);
}


TEST_CASE("test ExternalReference element","[externalreference]") {
  org::phenopackets::schema::v1::core::ExternalReference extrefpb;

  phenotools::ExternalReference er(extrefpb);
  // no id, this is an error
  vector<phenotools::Validation> validation = er.validate();
  REQUIRE(validation.size()==1);
  phenotools::Validation v1 = validation.at(0);
  REQUIRE(v1.is_error()==true);
  REQUIRE(v1.get_cause() == phenotools::ValidationCause::EXTERNAL_REFERENCE_LACKS_ID);
  // set the id. Then the element is OK.
  extrefpb.set_id("PMID:1234");
  phenotools::ExternalReference er2(extrefpb);
  validation = er2.validate();
  REQUIRE(validation.size()==0);
}


TEST_CASE("test Evidence","[evidence]") {

  org::phenopackets::schema::v1::core::Evidence evipb;
  // error -- Evidence needs to have an Ontology class evidence release_evidence_code

  phenotools::Evidence evi1(evipb);
  vector<phenotools::Validation> validation = evi1.validate();
  REQUIRE(validation.size()==1);
  phenotools::Validation v1 = validation.at(0);
  REQUIRE(v1.is_error()==true);
  REQUIRE(v1.get_cause() == phenotools::ValidationCause::EVIDENCE_LACKS_CODE);

  string eco_id = "ECO:0006017";
  string eco_label = "author statement from published clinical study used in manual assertion";

  org::phenopackets::schema::v1::core::OntologyClass* eco =
    google::protobuf::Arena::Create<org::phenopackets::schema::v1::core::OntologyClass>(&arena);
  eco->set_id(eco_id);
  eco->set_label(eco_label);
  // now add an ontology term -- all should be OK
  evipb.set_allocated_evidence_code(eco);
  phenotools::Evidence evi2(evipb);
  validation = evi2.validate();
  REQUIRE(validation.size()==0);
  evipb.release_evidence_code();
}

TEST_CASE("test Gene","[gene]") {
  org::phenopackets::schema::v1::core::Gene genepb;
  // error -- no id or symbol
  phenotools::Gene g1(genepb);
  vector<phenotools::Validation> validation = g1.validate();
  REQUIRE(validation.size()==2);
  phenotools::Validation v1 = validation.at(0);
  REQUIRE(v1.is_error()==true);
  REQUIRE(v1.get_cause() == phenotools::ValidationCause::GENE_LACKS_ID);
  phenotools::Validation v2 = validation.at(1);
  REQUIRE(v2.is_error()==true);
  REQUIRE(v2.get_cause() == phenotools::ValidationCause::GENE_LACKS_SYMBOL);
  // error -- have id but no symbol
  genepb.set_id("ENTREZ:7547");
  phenotools::Gene g2(genepb);
  validation = g2.validate();
  REQUIRE(validation.size()==1);
  v1 = validation.at(0);
  REQUIRE(v1.is_error()==true);
  REQUIRE(v1.get_cause() == phenotools::ValidationCause::GENE_LACKS_SYMBOL);
  // fix everything
  genepb.set_symbol("ZIC3");
  phenotools::Gene g3{genepb};
  validation = g3.validate();
  REQUIRE(validation.empty()==true);
}


TEST_CASE("Test HGVS allele","[hgvsallele]"){
  org::phenopackets::schema::v1::core::HgvsAllele hgvspb;
  // error -- no id or hgvs
  phenotools::HgvsAllele allele1(hgvspb);
  vector<phenotools::Validation> validation = allele1.validate();
  REQUIRE(validation.size()==2);
  phenotools::Validation v1 = validation.at(0);
  REQUIRE(v1.is_warning()==true);
  REQUIRE(v1.get_cause() == phenotools::ValidationCause::ALLELE_LACKS_ID);
  v1 = validation.at(1);
  REQUIRE(v1.is_error()==true);
  REQUIRE(v1.get_cause() == phenotools::ValidationCause::ALLELE_LACKS_HGVS);
  // add the HGVS -- there should be only a warning now
  hgvspb.set_hgvs("NM_000276.3:c.2581G>A");
  phenotools::HgvsAllele allele2(hgvspb);
  validation = allele2.validate();
  REQUIRE(validation.size()==1);
  v1 = validation.at(0);
  REQUIRE(v1.is_warning()==true);
  REQUIRE(v1.get_cause() == phenotools::ValidationCause::ALLELE_LACKS_ID);
  // add an ID -- there should now be no warning
  hgvspb.set_id("id:A");
  phenotools::HgvsAllele allele3(hgvspb);
  validation = allele3.validate();
  REQUIRE(validation.empty()==true);
}


TEST_CASE("Test VcfAllele","[vcfallele]") {
  org::phenopackets::schema::v1::core::VcfAllele vcfpb;
  // error -- no data
  phenotools::VcfAllele allele1(vcfpb);
  vector<phenotools::Validation> validation = allele1.validate();
  REQUIRE(validation.size()==5);
  phenotools::Validation v = validation.at(0);
  REQUIRE(v.is_warning()==true);
  REQUIRE(v.get_cause() == phenotools::ValidationCause::ALLELE_LACKS_ID);
  v = validation.at(1);
  REQUIRE(v.is_error()==true);
  REQUIRE(v.get_cause() == phenotools::ValidationCause::LACKS_GENOME_ASSEMBLY);
  v = validation.at(2);
  REQUIRE(v.is_error()==true);
  REQUIRE(v.get_cause() == phenotools::ValidationCause::LACKS_CHROMOSOME);
  v = validation.at(3);
  REQUIRE(v.is_error()==true);
  REQUIRE(v.get_cause() == phenotools::ValidationCause::LACKS_REF);
  v = validation.at(4);
  REQUIRE(v.is_error()==true);
  REQUIRE(v.get_cause() == phenotools::ValidationCause::LACKS_ALT);
  // Now let's make a well formed VCF
  vcfpb.set_id("id:A");
  vcfpb.set_genome_assembly("GRCh37");
  vcfpb.set_chr("chr1");
  vcfpb.set_pos(42);
  vcfpb.set_ref("A");
  vcfpb.set_alt("C");
  phenotools::VcfAllele allele2(vcfpb);
  validation = allele2.validate();
  REQUIRE(validation.empty()==true);
}


TEST_CASE("Test Variant","[variant]"){
  org::phenopackets::schema::v1::core::Variant variantpb;
  // error -- no data
  phenotools::Variant var1(variantpb);
  vector<phenotools::Validation> validation = var1.validate();
  REQUIRE(validation.size()==2);
  phenotools::Validation v = validation.at(0);
  REQUIRE(v.is_error()==true);
  REQUIRE(v.get_cause() == phenotools::ValidationCause::LACKS_ALLELE);
  v = validation.at(1);
  REQUIRE(v.is_error()==true);
  REQUIRE(v.get_cause() == phenotools::ValidationCause::LACKS_ZYGOSITY);
  // now construct a correct variant
  org::phenopackets::schema::v1::core::VcfAllele* vc =
    google::protobuf::Arena::Create<org::phenopackets::schema::v1::core::VcfAllele>(&arena);
  vc->set_id("id:A");
  vc->set_genome_assembly("GRCh37");
  vc->set_chr("chr1");
  vc->set_pos(42);
  vc->set_ref("A");
  vc->set_alt("C");

  // Now make an Age that has an ontology class
  string id = "GENO:0000135";
  string label = "heterozygous";
  org::phenopackets::schema::v1::core::OntologyClass* c =
    google::protobuf::Arena::Create<org::phenopackets::schema::v1::core::OntologyClass>(&arena);
  c->set_id(id);
  c->set_label(label);

  variantpb.set_allocated_vcf_allele(vc);
  variantpb.set_allocated_zygosity(c);

  phenotools::Variant var2(variantpb);
  validation = var2.validate();
  REQUIRE(validation.empty()==true); // we expect no errors/warnings here.

  variantpb.release_vcf_allele();
  variantpb.release_zygosity();
}


TEST_CASE("Test Disease","[disease]"){
  org::phenopackets::schema::v1::core::Disease diseasepb;
  // error -- no data
  phenotools::Disease d1(diseasepb);
  vector<phenotools::Validation> validation = d1.validate();
  REQUIRE(validation.size()==1);
  phenotools::Validation v = validation.at(0);
  REQUIRE(v.is_error()==true);
  REQUIRE(v.get_cause() == phenotools::ValidationCause::DISEASE_LACKS_TERM);

  // add a term and an onset with one error
  string id = "OMIM:101600";
  string label = "Pfeiffer syndrome";
  org::phenopackets::schema::v1::core::OntologyClass* diseaseTerm =
    google::protobuf::Arena::Create<org::phenopackets::schema::v1::core::OntologyClass>(&arena);
  diseaseTerm->set_id(id);
  diseaseTerm->set_label(label);

  org::phenopackets::schema::v1::core::OntologyClass* onsetTerm =
    google::protobuf::Arena::Create<org::phenopackets::schema::v1::core::OntologyClass>(&arena);
  onsetTerm->set_id("HP:0011463");
  // Forgot to set label  Childhood onset HP:0011463

  diseasepb.set_allocated_term(diseaseTerm);
  diseasepb.set_allocated_class_of_onset(onsetTerm);
  phenotools::Disease d2(diseasepb);
  validation = d2.validate();
  REQUIRE(validation.size()==1);
  v = validation.at(0);
  REQUIRE(v.is_error()==true);
  REQUIRE(v.get_cause() == phenotools::ValidationCause::ONTOLOGY_LABEL_EMPTY);

  diseasepb.release_term();
  diseasepb.release_class_of_onset();
}


TEST_CASE("Test Resource","[resource]") {
  org::phenopackets::schema::v1::core::Resource resourcepb;
  // error -- no data
  phenotools::Resource r1(resourcepb);
  vector<phenotools::Validation> validation = r1.validate();
  REQUIRE(validation.size()==6);
  phenotools::Validation v = validation.at(0);
  REQUIRE(v.is_error());
  REQUIRE(v.get_cause() == phenotools::ValidationCause::RESOURCE_LACKS_ID);
  v = validation.at(1);
  REQUIRE(v.is_error());
  REQUIRE(v.get_cause() == phenotools::ValidationCause::RESOURCE_LACKS_NAME);
  v = validation.at(2);
  REQUIRE(v.is_error());
  REQUIRE(v.get_cause() == phenotools::ValidationCause::RESOURCE_LACKS_NAMESPACE_PREFIX);
  v = validation.at(3);
  REQUIRE(v.is_error());
  REQUIRE(v.get_cause() == phenotools::ValidationCause::RESOURCE_LACKS_URL);
  v = validation.at(4);
  REQUIRE(v.is_error());
  REQUIRE(v.get_cause() == phenotools::ValidationCause::RESOURCE_LACKS_VERSION);
  v = validation.at(5);
  REQUIRE(v.is_error());
  REQUIRE(v.get_cause() == phenotools::ValidationCause::RESOURCE_LACKS_IRI_PREFIX);

  resourcepb.set_id("hp");
  resourcepb.set_name("human phenotype ontology");
  resourcepb.set_namespace_prefix("HP");
  resourcepb.set_url("http://purl.obolibrary.org/obo/hp.owl");
  resourcepb.set_version("2018-03-08");
  resourcepb.set_iri_prefix("http://purl.obolibrary.org/obo/HP_");
  phenotools::Resource r2(resourcepb);
  validation = r2.validate();
  REQUIRE(validation.empty());
}




TEST_CASE("Procedure","[procedure]") {
  org::phenopackets::schema::v1::core::Procedure procedurepb;
  // error -- no data
  phenotools::Procedure p1(procedurepb);
  vector<phenotools::Validation> validation = p1.validate();
  REQUIRE(validation.size()==1);
  phenotools::Validation v = validation.at(0);
  REQUIRE(v.is_error());
  REQUIRE(v.get_cause() == phenotools::ValidationCause::PROCEDURE_LACKS_CODE);
  string id = "NCIT:C28743";
  string label = "Punch Biopsy";
  org::phenopackets::schema::v1::core::OntologyClass* code =
    google::protobuf::Arena::Create<org::phenopackets::schema::v1::core::OntologyClass>(&arena);
  code->set_id(id);
  code->set_label(label);
  procedurepb.set_allocated_code(code);
  phenotools::Procedure p2(procedurepb);
  validation = p2.validate();
  REQUIRE(validation.empty());
  procedurepb.release_code();
}

TEST_CASE("Parse hp.small.json","[parse_hp_small_json]")
{
  string hp_json_path = "../testdata/hp.small.json";
  {
     std::ifstream infile(hp_json_path);
     if (!infile.good()) {
       cerr << "\n[ERROR] Could not find hp_json test file: "
	    << hp_json_path << "\n";
       exit(1);
     }
  }
  JsonOboParser parser {hp_json_path};
  std::unique_ptr<Ontology> ontology = parser.get_ontology();
  //ontology->debug_print();
  TermId t1 = TermId::from_string("HP:0000002");
  std::optional<Term> t1opt = ontology->get_term(t1);
  REQUIRE(t1opt);
  Term term = *t1opt;
  REQUIRE("Fake term 2" == term.get_label());
  vector<TermId> parents = ontology->get_isa_parents(t1);
  REQUIRE(1 == parents.size());
  TermId par = TermId::from_string("HP:0000001");
  REQUIRE(par == parents.at(0));
  string def = "Definition for HP:0000002.";
  REQUIRE(def ==  term.get_definition());
  
  vector<Xref> def_xrefs = term.get_definition_xref_list();
  REQUIRE(1 == def_xrefs.size());
  Xref dxref1 = def_xrefs.at(0);
  TermId expectedXref = TermId::from_string("HPO:fake2");
  REQUIRE(expectedXref == dxref1.get_termid());
  vector<PredicateValue> provals = term.get_property_values();
  REQUIRE(3 == provals.size());
  PredicateValue proval_a = provals.at(0);
  REQUIRE(Predicate::HAS_OBO_NAMESPACE == proval_a.get_property());
  REQUIRE("human_phenotype" == proval_a.get_value());
  vector<Xref> xrefs = term.get_term_xref_list();
  REQUIRE(1 == xrefs.size());
  Xref xref1 = xrefs.at(0);
  TermId expectedXreft = TermId::from_string("UMLS:C4024355");
  REQUIRE(expectedXreft == xref1.get_termid());
  // Term 4 has a synonym
  TermId t4 = TermId::from_string("HP:0000004");
  std::optional<Term> t4opt = ontology->get_term(t4);
  REQUIRE(t4opt);
  Term term4 = *t4opt;
  vector<Synonym> synonyms = term4.get_synonyms();
  REQUIRE(1 == synonyms.size());
  Synonym s1 = synonyms.at(0);
  REQUIRE(s1.is_exact());
  REQUIRE("Abnormal shape of thyroid gland" == s1.get_label());
}


TEST_CASE("Parse Phenopacket with bad term","[has_term_not_in_ontology]") {
  string hp_json_path = "../testdata/hp.small.json";
  JsonOboParser parser {hp_json_path};
  std::unique_ptr<Ontology>  ontology = parser.get_ontology();
  string phenopacket_path = "../testdata/small-phenopacket-2.json";
  std::ifstream inFile;
  inFile.open ( phenopacket_path );
  if ( ! inFile.good() ) {
    cerr << "Could not open Phenopacket file at " << phenopacket_path <<"\n";
     exit(1);
  }
  std::stringstream sstr;
  sstr << inFile.rdbuf();
  string JSONstring = sstr.str();
  ::google::protobuf::util::JsonParseOptions options;
  ::org::phenopackets::schema::v1::Phenopacket phenopacketpb;
  ::google::protobuf::util::JsonStringToMessage (JSONstring, &phenopacketpb, options);
  phenotools::Phenopacket ppacket(phenopacketpb);
  // now test for a "bad" term HP:9999999, which is not in the ontology
  vector<phenotools::Validation> validation = ppacket.semantically_validate(ontology);
  // there are 5 issues. One is the bad term, and 4 relate to warnings about
  // metadata ontologies not used in the rest of the phenopacket
  REQUIRE(5 == validation.size());
  phenotools::Validation v1 = validation.at(0);
  REQUIRE(v1.get_cause() == phenotools::ValidationCause::UNRECOGNIZED_TERMID);
  string msg = "[ERROR] Could not find HP:9999999 in the ontology";
  REQUIRE(msg == v1.message());
  for (auto i = 1; i<5; i++) {
    v1 = validation.at(i);
    //REQUIRE(v1.is_warning());
    //REQUIRE(v1.get_cause() ==  phenotools::ValidationCause::METADATA_HAS_SUPERFLUOUS_ONTOLOGY );
    cout << v1 << "\n";
  }
}
