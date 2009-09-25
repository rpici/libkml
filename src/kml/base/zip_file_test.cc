// Copyright 2009, Google Inc. All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//  1. Redistributions of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//  2. Redistributions in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//  3. Neither the name of Google Inc. nor the names of its contributors may be
//     used to endorse or promote products derived from this software without
//     specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
// WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
// EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
// OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
// WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
// OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
// ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

// This file contains the unit tests for the ZipFile class.

#include "kml/base/zip_file.h"
#include "boost/scoped_ptr.hpp"
#include "kml/base/file.h"
#include "kml/base/tempfile.h"
#include "gtest/gtest.h"

// The following define is a convenience for testing inside Google.
#ifdef GOOGLE_INTERNAL
#include "kml/base/google_internal_test.h"
#endif

#ifndef DATADIR
#error *** DATADIR must be defined! ***
#endif

namespace kmlbase {

class ZipFileTest : public testing::Test {
 protected:
  boost::scoped_ptr<ZipFile> zip_file_;
};

TEST_F(ZipFileTest, TestOpenFromString) {
  // doc.kmz contains doc.kml and is a valid zip archive.
  const std::string kGoodKmz = std::string(DATADIR) + "/kmz/doc.kmz";
  std::string zip_file_data;
  ASSERT_TRUE(File::ReadFileToString(kGoodKmz.c_str(), &zip_file_data));
  ASSERT_FALSE(zip_file_data.empty());
  zip_file_.reset(ZipFile::OpenFromString(zip_file_data));
  ASSERT_TRUE(zip_file_);
  std::string kml_data;
  // doc.kml can be read.
  ASSERT_TRUE(zip_file_->FindFirstOf(".kml", &kml_data));
  ASSERT_FALSE(kml_data.empty());
  // nokml.kmz is a valid zip archive, but does not contain any KML files
  const std::string kBadKmz = std::string(DATADIR) + "/kmz/nokml.kmz";
  zip_file_data.clear();
  ASSERT_TRUE(File::ReadFileToString(kBadKmz.c_str(), &zip_file_data));
  ASSERT_FALSE(zip_file_data.empty());
  zip_file_.reset(ZipFile::OpenFromString(zip_file_data));
  ASSERT_TRUE(zip_file_);
  kml_data.clear();
  // There is no KML file to read.
  ASSERT_FALSE(zip_file_->FindFirstOf(".kml", &kml_data));
  ASSERT_TRUE(kml_data.empty());
}

TEST_F(ZipFileTest, TestOpenFromFile) {
  // doc.kmz contains doc.kml and is a valid zip archive.
  const std::string kGoodKmz = std::string(DATADIR) + "/kmz/doc.kmz";
  zip_file_.reset(ZipFile::OpenFromFile(kGoodKmz.c_str()));
  ASSERT_TRUE(zip_file_);
  std::string kml_data;
  // doc.kml can be read.
  ASSERT_TRUE(zip_file_->FindFirstOf(".kml", &kml_data));
  ASSERT_FALSE(kml_data.empty());
  // nokml.kmz is a valid zip archive, but does not contain any KML files
  const std::string kBadKmz = std::string(DATADIR) + "/kmz/nokml.kmz";
  zip_file_.reset(ZipFile::OpenFromFile(kBadKmz.c_str()));
  ASSERT_TRUE(zip_file_);
  kml_data.clear();
  // There is no KML file to read.
  ASSERT_FALSE(zip_file_->FindFirstOf(".kml", &kml_data));
  ASSERT_TRUE(kml_data.empty());
}

TEST_F(ZipFileTest, TestOpenFromBadFile) {
  // Two kinds of bad file.
  // 1: a non-existant file:
  const std::string kNoSuchFile("nosuchfile.kmz");
  zip_file_.reset(ZipFile::OpenFromFile(kNoSuchFile.c_str()));
  // The file cannot be opened.
  ASSERT_TRUE(zip_file_ == NULL);
  // 2: a file that is not a valid KMZ archive.
  const std::string kBadKmz= std::string(DATADIR) + "/kmz/bad.kmz";
  zip_file_.reset(ZipFile::OpenFromFile(kBadKmz.c_str()));
  // The file could not be read.
  ASSERT_TRUE(zip_file_ == NULL);
}

TEST_F(ZipFileTest, TestCreate) {
  // Create a temp file into which we'll write our KMZ data.
  kmlbase::TempFilePtr tempfile = kmlbase::TempFile::CreateTempFile();
  ASSERT_TRUE(tempfile != NULL);
  // Create a KMZ file containing a KML file that is a placemark called
  // 'tmp kml'.
  const std::string kKml("<Placemark><name>tmp kml</name></Placemark>");
  ASSERT_TRUE(ZipFile::Create(tempfile->name().c_str()));
  // Now read the file, ensuring it was properly written.
  ASSERT_TRUE(File::Exists(tempfile->name()));
}

TEST_F(ZipFileTest, TestIsZipData) {
  // Verify that a valid KMZ archive passes IsKmz().
  const std::string kGoodKmz = std::string(DATADIR) + "/kmz/doc.kmz";
  std::string kmz_data;
  File::ReadFileToString(kGoodKmz, &kmz_data);
  ASSERT_FALSE(kmz_data.empty());
  ASSERT_TRUE(ZipFile::IsZipData(kmz_data));

  // Verify that an invalid KMZ archive fails IsKmz().
  const std::string kBadKmz = std::string(DATADIR) + "/kmz/bad.kmz";
  kmz_data.clear();
  File::ReadFileToString(kBadKmz, &kmz_data);
  ASSERT_FALSE(kmz_data.empty());
  ASSERT_FALSE(ZipFile::IsZipData(kmz_data));
}

TEST_F(ZipFileTest, TestFindFirstOf) {
  const std::string kGoodKmz = std::string(DATADIR) + "/kmz/doc.kmz";
  zip_file_.reset(ZipFile::OpenFromFile(kGoodKmz.c_str()));
  ASSERT_TRUE(zip_file_);
  std::string kml_data;
  ASSERT_FALSE(zip_file_->FindFirstOf(".bad", &kml_data));
  ASSERT_TRUE(kml_data.empty());
  ASSERT_TRUE(zip_file_->FindFirstOf(".kml", &kml_data));
}

TEST_F(ZipFileTest, TestGetToc) {
  // multikml-nodoc.kmz has three kml files added in the following order:
  // - z/c.kml
  // - b.kml
  // - a/a.kml
  const std::string kMulti1 = std::string(DATADIR) + "/kmz/multikml-nodoc.kmz";
  zip_file_.reset(ZipFile::OpenFromFile(kMulti1.c_str()));
  ASSERT_TRUE(zip_file_);
  StringVector list;
  zip_file_->GetToc(&list);
  // 3 files were read into the vector.
  ASSERT_TRUE(3 == list.size());
  // They appear in the same order in which they were added.
  ASSERT_EQ(std::string("z/c.kml"), list[0]);
  ASSERT_EQ(std::string("b.kml"), list[1]);
  ASSERT_EQ(std::string("a/a.kml"), list[2]);
}

TEST_F(ZipFileTest, TestIsInToc) {
  const std::string kGoodKmz = std::string(DATADIR) + "/kmz/doc.kmz";
  zip_file_.reset(ZipFile::OpenFromFile(kGoodKmz.c_str()));
  ASSERT_TRUE(zip_file_);
  ASSERT_TRUE(zip_file_->IsInToc("doc.kml"));
  ASSERT_FALSE(zip_file_->IsInToc("docx.kml"));
}

TEST_F(ZipFileTest, TestGetEntry) {
  // nokml.kmz has a file called foo.txt in a folder called foo.
  const std::string kNokml = std::string(DATADIR) + "/kmz/nokml.kmz";
  zip_file_.reset(ZipFile::OpenFromFile(kNokml.c_str()));
  ASSERT_TRUE(zip_file_);
  std::string file_data;
  ASSERT_TRUE(zip_file_->GetEntry("foo/foo.txt", &file_data));
  ASSERT_FALSE(file_data.empty());
  std::string tmp = file_data;
  // But does not have a file called bar.txt in that folder
  ASSERT_FALSE(zip_file_->GetEntry("foo/bar.txt", &file_data));
  // The original data was untouched by this failure.
  ASSERT_FALSE(file_data.empty());
  ASSERT_EQ(tmp, file_data);
  // Assert we handle a NULL output string.
  ASSERT_FALSE(zip_file_->GetEntry("bar", NULL));
}

TEST_F(ZipFileTest, TestGetKmzData) {
  const std::string kGoodKmz = std::string(DATADIR) + "/kmz/doc.kmz";
  std::string kmz_data;
  File::ReadFileToString(kGoodKmz, &kmz_data);
  zip_file_.reset(ZipFile::OpenFromString(kmz_data));
  ASSERT_TRUE(zip_file_);
  ASSERT_EQ(kmz_data, zip_file_->get_data());
}

TEST_F(ZipFileTest, TestAddEntry) {
  TempFilePtr tempfile = TempFile::CreateTempFile();
  ASSERT_TRUE(tempfile != NULL);
  {
    // Create an empty ZipFile.
    boost::scoped_ptr<ZipFile> zipfile(
        ZipFile::Create(tempfile->name().c_str()));
    ASSERT_TRUE(zipfile.get());
    // Add three files to the archive.
    const std::string kNewKml = "<Placemark><name/></Placemark>";
    ASSERT_TRUE(zipfile->AddEntry(kNewKml, "doc.kml"));
    ASSERT_TRUE(zipfile->AddEntry(kNewKml, "files/new.kml"));
    ASSERT_TRUE(zipfile->AddEntry(kNewKml, "other/blah.kml"));
    // Fails because it points above the archive.
    ASSERT_FALSE(zipfile->AddEntry(kNewKml, "../invalid.kml"));
    // Fails because the path is absolute.
    ASSERT_FALSE(zipfile->AddEntry(kNewKml, "/also/invalid.kml"));
  }
  // ZipFile's destructor closes the file handle and cleans up.
  ASSERT_TRUE(File::Exists(tempfile->name().c_str()));

  // Verify that the archive we created contains the files in order.
  boost::scoped_ptr<ZipFile> created(
      ZipFile::OpenFromFile(tempfile->name().c_str()));
  ASSERT_TRUE(created.get());
  std::vector<std::string> list;
  created->GetToc(&list);
  ASSERT_EQ(static_cast<size_t>(3), list.size());
  ASSERT_EQ(std::string("doc.kml"), list[0]);
  ASSERT_EQ(std::string("files/new.kml"), list[1]);
  ASSERT_EQ(std::string("other/blah.kml"), list[2]);
}

TEST_F(ZipFileTest, TestAddEntryDupe) {
  // Assert that calling AddEntry on the same path with new content does not
  // overwrite the old content.
  TempFilePtr tempfile = TempFile::CreateTempFile();
  ASSERT_TRUE(tempfile != NULL);
  {
    boost::scoped_ptr<ZipFile> zipfile(
        ZipFile::Create(tempfile->name().c_str()));
    ASSERT_TRUE(zipfile.get());
    const std::string kKml = "<Placemark><name/></Placemark>";
    ASSERT_TRUE(zipfile->AddEntry(kKml, "doc.kml"));
    const std::string kNewKml = "<Document><name/></Document>";
    ASSERT_TRUE(zipfile->AddEntry(kNewKml, "doc.kml"));
  }
  ASSERT_TRUE(File::Exists(tempfile->name().c_str()));
  boost::scoped_ptr<ZipFile> created(
      ZipFile::OpenFromFile(tempfile->name().c_str()));
  ASSERT_TRUE(created.get());
  std::string read_kml;
  ASSERT_TRUE(created->GetEntry("doc.kml", &read_kml));
  const std::string kExpectedKml= "<Placemark><name/></Placemark>";
  ASSERT_EQ(kExpectedKml, read_kml);
}

TEST_F(ZipFileTest, TestAddEntryBad) {
  // AddEntry should only be called on a ZipFile object created by
  // ZipFile::Create. This test asserts sane behavior when OpenFromString
  // is used instead.
  const std::string kGoodKmz = std::string(DATADIR) + "/kmz/doc.kmz";
  std::string zip_file_data;
  ASSERT_TRUE(File::ReadFileToString(kGoodKmz.c_str(), &zip_file_data));
  ASSERT_FALSE(zip_file_data.empty());
  zip_file_.reset(ZipFile::OpenFromString(zip_file_data));
  ASSERT_TRUE(zip_file_.get());
  const std::string kNewKml = "<Placemark><name/></Placemark>";
  ASSERT_FALSE(zip_file_->AddEntry(kNewKml, "doc.kml"));
}

TEST_F(ZipFileTest, TestBadPkZipData) {
  // Some ZIP files created with new zip-creation tools can't be uncompressed
  // by our underlying minizip library. Assert sane behavior.
  const std::string kBadKmz= std::string(DATADIR) + "/kmz/bad-pk-data.kmz";
  std::string zip_file_data;
  ASSERT_TRUE(File::ReadFileToString(kBadKmz.c_str(), &zip_file_data));
  ASSERT_FALSE(zip_file_data.empty());
  zip_file_.reset(ZipFile::OpenFromString(zip_file_data));
  ASSERT_FALSE(zip_file_->GetEntry("doc.kml", NULL));
}

}  // end namespace kmlbase

int main(int argc, char** argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
