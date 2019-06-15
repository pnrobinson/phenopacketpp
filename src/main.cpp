#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <memory>

#include <google/protobuf/message.h>
#include <google/protobuf/util/json_util.h>

#include "phenopackets.pb.h"
#include "base.pb.h"
#include "phenotools.h"


using namespace std;



int main(int argc, char ** argv) {
  if (argc!=2) {
    cerr << "usage: ./phenotools phenopacket-file.json\n";
    exit(EXIT_FAILURE);
  }
  string fileName=argv[1];

  GOOGLE_PROTOBUF_VERIFY_VERSION;

  stringstream sstr;
  ifstream inFile;
  inFile.open(fileName);
  if (! inFile.good()) {
     cerr << "Could not open Phenopacket file at " << fileName <<"\n";
     return EXIT_FAILURE;
   }
   sstr << inFile.rdbuf();
   string JSONstring = sstr.str();
   //cout <<"[INFO] reading phenopacket\n" << JSONstring << "\n";

  ::google::protobuf::util::JsonParseOptions options;
  ::org::phenopackets::schema::v1::Phenopacket phenopacketpb;
  ::google::protobuf::util::JsonStringToMessage(JSONstring,&phenopacketpb,options);
  cout << "Phenopacket at: " << fileName << "\n";

  auto ppacket = std::make_unique<Phenopacket>(phenopacketpb);
  // cout << "\tsubject.id: "<<phenopacket.subject().id() << "\n";
  // print age if available
  

}
