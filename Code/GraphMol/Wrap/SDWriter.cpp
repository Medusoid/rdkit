//
//  Copyright (C) 2003-2021 Greg Landrum and Rational Discovery LLC
//
//   @@ All Rights Reserved @@
//  This file is part of the RDKit.
//  The contents are covered by the terms of the BSD license
//  which is included in the file license.txt, found at the root
//  of the RDKit source tree.
//

#define NO_IMPORT_ARRAY
#include <RDBoost/python.h>
#include <string>

// ours
#include <GraphMol/FileParsers/MolWriters.h>
#include <GraphMol/RDKitBase.h>
#include "rdchem.h"
#include "ContextManagers.h"
#include <RDBoost/PySequenceHolder.h>
#include <RDBoost/python_streambuf.h>

namespace python = boost::python;
using boost_adaptbx::python::streambuf;

namespace RDKit {
SDWriter *getSDWriter(python::object &fileobj) {
  auto *sb = new streambuf(fileobj, 't');
  auto *ost = new streambuf::ostream(sb);
  return new SDWriter(ost, true);
}

void SetSDWriterProps(SDWriter &writer, python::object props) {
  // convert the python list to a STR_VECT
  STR_VECT propNames;
  PySequenceHolder<std::string> seq(props);
  for (unsigned int i = 0; i < seq.size(); i++) {
    propNames.push_back(seq[i]);
  }
  writer.setProps(propNames);
}
void WriteMolToSD(SDWriter &writer, ROMol &mol, int confId) {
  writer.write(mol, confId);
}

std::string getSDTextHelper(const ROMol &mol, int confId, bool kekulize,
                            bool force_V3000, int molid) {
  return SDWriter::getText(mol, confId, kekulize, force_V3000, molid);
}

struct sdwriter_wrap {
  static void wrap() {
    std::string docStr =
        "A class for writing molecules to SD files.\n\
\n\
  Usage examples:\n\
\n\
    1) writing to a named file:\n\n\
       >>> writer = SDWriter('out.sdf')\n\
       >>> for mol in list_of_mols:\n\
       ...    writer.write(mol)\n\
\n\
    2) writing to a file-like object: \n\n\
       >>> import gzip\n\
       >>> outf=gzip.open('out.sdf.gz','wt+')\n\
       >>> writer = SDWriter(outf)\n\
       >>> for mol in list_of_mols:\n\
       ...   writer.write(mol)\n\
       >>> writer.close()\n\
       >>> outf.close()\n\
\n\
  By default all non-private molecular properties are written to the SD file.\n\
  This can be changed using the SetProps method:\n\n\
       >>> writer = SDWriter('out.sdf')\n\
       >>> writer.SetProps(['prop1','prop2'])\n\
\n";
    python::class_<SDWriter, boost::noncopyable>("SDWriter", docStr.c_str(),
                                                 python::no_init)
        .def("__init__", python::make_constructor(&getSDWriter))
        .def(python::init<std::string>(python::args("self", "fileName"),
                                       "Constructor.\n\n"
                                       "   If a string argument is provided, "
                                       "it will be treated as the name of the "
                                       "output file.\n"
                                       "   If a file-like object is provided, "
                                       "output will be sent there.\n\n"))
        .def("__enter__", &MolIOEnter<SDWriter>,
             python::return_internal_reference<>())
        .def("__exit__", &MolIOExit<SDWriter>)
        .def("SetProps", SetSDWriterProps, python::args("self", "props"),
             "Sets the properties to be written to the output file\n\n"
             "  ARGUMENTS:\n\n"
             "    - props: a list or tuple of property names\n\n")
        .def("write", WriteMolToSD,
             (python::arg("self"), python::arg("mol"),
              python::arg("confId") = -1),
             "Writes a molecule to the output file.\n\n"
             "  ARGUMENTS:\n\n"
             "    - mol: the Mol to be written\n"
             "    - confId: (optional) ID of the conformation to write\n\n")
        .def("flush", &SDWriter::flush, python::args("self"),
             "Flushes the output file (forces the disk file to be "
             "updated).\n\n")
        .def("close", &SDWriter::close, python::args("self"),
             "Flushes the output file and closes it. The Writer cannot be used "
             "after this.\n\n")
        .def("NumMols", &SDWriter::numMols, python::args("self"),
             "Returns the number of molecules written so far.\n\n")
        .def("SetForceV3000", &SDWriter::setForceV3000,
             python::args("self", "val"),
             "Sets whether or not V3000 mol file writing is being forced.\n\n")
        .def("GetForceV3000", &SDWriter::getForceV3000, python::args("self"),
             "Returns whether or not V3000 mol file writing is being "
             "forced.\n\n")
        .def("SetKekulize", &SDWriter::setKekulize, python::args("self", "val"),
             "Sets whether or not molecules are kekulized on writing.\n\n")
        .def("GetKekulize", &SDWriter::getKekulize, python::args("self"),
             "Returns whether or not molecules are kekulized on writing.\n\n")
        .def("GetText", getSDTextHelper,
             (python::arg("mol"), python::arg("confId") = -1,
              python::arg("kekulize") = true,
              python::arg("force_v3000") = false, python::arg("molid") = -1),
             "returns the SD text for a molecule")
        .staticmethod("GetText");
  };
};
}  // namespace RDKit

void wrap_sdwriter() { RDKit::sdwriter_wrap::wrap(); }
