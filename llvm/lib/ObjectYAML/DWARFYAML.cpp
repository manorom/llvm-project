//===- DWARFYAML.cpp - DWARF YAMLIO implementation ------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file defines classes for handling the YAML representation of DWARF Debug
// Info.
//
//===----------------------------------------------------------------------===//

#include "llvm/ObjectYAML/DWARFYAML.h"
#include "llvm/BinaryFormat/Dwarf.h"

namespace llvm {

bool DWARFYAML::Data::isEmpty() const {
  return DebugStrings.empty() && AbbrevDecls.empty() && !DebugAranges &&
         DebugRanges.empty() && !PubNames && !PubTypes && !GNUPubNames &&
         !GNUPubTypes && CompileUnits.empty() && DebugLines.empty();
}

SetVector<StringRef> DWARFYAML::Data::getNonEmptySectionNames() const {
  SetVector<StringRef> SecNames;
  if (!DebugStrings.empty())
    SecNames.insert("debug_str");
  if (DebugAranges)
    SecNames.insert("debug_aranges");
  if (!DebugRanges.empty())
    SecNames.insert("debug_ranges");
  if (!DebugLines.empty())
    SecNames.insert("debug_line");
  if (!DebugAddr.empty())
    SecNames.insert("debug_addr");
  if (!AbbrevDecls.empty())
    SecNames.insert("debug_abbrev");
  if (!CompileUnits.empty())
    SecNames.insert("debug_info");
  if (PubNames)
    SecNames.insert("debug_pubnames");
  if (PubTypes)
    SecNames.insert("debug_pubtypes");
  if (GNUPubNames)
    SecNames.insert("debug_gnu_pubnames");
  if (GNUPubTypes)
    SecNames.insert("debug_gnu_pubtypes");
  if (DebugStrOffsets)
    SecNames.insert("debug_str_offsets");
  if (DebugRnglists)
    SecNames.insert("debug_rnglists");
  if (DebugLoclists)
    SecNames.insert("debug_loclists");
  return SecNames;
}

namespace yaml {

void MappingTraits<DWARFYAML::Data>::mapping(IO &IO, DWARFYAML::Data &DWARF) {
  void *OldContext = IO.getContext();
  DWARFYAML::DWARFContext DWARFCtx;
  IO.setContext(&DWARFCtx);
  IO.mapOptional("debug_str", DWARF.DebugStrings);
  IO.mapOptional("debug_abbrev", DWARF.AbbrevDecls);
  IO.mapOptional("debug_aranges", DWARF.DebugAranges);
  if (!DWARF.DebugRanges.empty() || !IO.outputting())
    IO.mapOptional("debug_ranges", DWARF.DebugRanges);
  IO.mapOptional("debug_pubnames", DWARF.PubNames);
  IO.mapOptional("debug_pubtypes", DWARF.PubTypes);
  DWARFCtx.IsGNUPubSec = true;
  IO.mapOptional("debug_gnu_pubnames", DWARF.GNUPubNames);
  IO.mapOptional("debug_gnu_pubtypes", DWARF.GNUPubTypes);
  IO.mapOptional("debug_info", DWARF.CompileUnits);
  IO.mapOptional("debug_line", DWARF.DebugLines);
  IO.mapOptional("debug_addr", DWARF.DebugAddr);
  IO.mapOptional("debug_str_offsets", DWARF.DebugStrOffsets);
  IO.mapOptional("debug_rnglists", DWARF.DebugRnglists);
  IO.mapOptional("debug_loclists", DWARF.DebugLoclists);
  IO.setContext(OldContext);
}

void MappingTraits<DWARFYAML::Abbrev>::mapping(IO &IO,
                                               DWARFYAML::Abbrev &Abbrev) {
  IO.mapOptional("Code", Abbrev.Code);
  IO.mapRequired("Tag", Abbrev.Tag);
  IO.mapRequired("Children", Abbrev.Children);
  IO.mapRequired("Attributes", Abbrev.Attributes);
}

void MappingTraits<DWARFYAML::AttributeAbbrev>::mapping(
    IO &IO, DWARFYAML::AttributeAbbrev &AttAbbrev) {
  IO.mapRequired("Attribute", AttAbbrev.Attribute);
  IO.mapRequired("Form", AttAbbrev.Form);
  if(AttAbbrev.Form == dwarf::DW_FORM_implicit_const)
    IO.mapRequired("Value", AttAbbrev.Value);
}

void MappingTraits<DWARFYAML::ARangeDescriptor>::mapping(
    IO &IO, DWARFYAML::ARangeDescriptor &Descriptor) {
  IO.mapRequired("Address", Descriptor.Address);
  IO.mapRequired("Length", Descriptor.Length);
}

void MappingTraits<DWARFYAML::ARange>::mapping(IO &IO,
                                               DWARFYAML::ARange &ARange) {
  IO.mapOptional("Format", ARange.Format, dwarf::DWARF32);
  IO.mapOptional("Length", ARange.Length);
  IO.mapRequired("Version", ARange.Version);
  IO.mapRequired("CuOffset", ARange.CuOffset);
  IO.mapOptional("AddressSize", ARange.AddrSize);
  IO.mapOptional("SegmentSelectorSize", ARange.SegSize, 0);
  IO.mapOptional("Descriptors", ARange.Descriptors);
}

void MappingTraits<DWARFYAML::RangeEntry>::mapping(
    IO &IO, DWARFYAML::RangeEntry &Descriptor) {
  IO.mapRequired("LowOffset", Descriptor.LowOffset);
  IO.mapRequired("HighOffset", Descriptor.HighOffset);
}

void MappingTraits<DWARFYAML::Ranges>::mapping(IO &IO,
                                               DWARFYAML::Ranges &DebugRanges) {
  IO.mapOptional("Offset", DebugRanges.Offset);
  IO.mapOptional("AddrSize", DebugRanges.AddrSize);
  IO.mapRequired("Entries", DebugRanges.Entries);
}

void MappingTraits<DWARFYAML::PubEntry>::mapping(IO &IO,
                                                 DWARFYAML::PubEntry &Entry) {
  IO.mapRequired("DieOffset", Entry.DieOffset);
  if (static_cast<DWARFYAML::DWARFContext *>(IO.getContext())->IsGNUPubSec)
    IO.mapRequired("Descriptor", Entry.Descriptor);
  IO.mapRequired("Name", Entry.Name);
}

void MappingTraits<DWARFYAML::PubSection>::mapping(
    IO &IO, DWARFYAML::PubSection &Section) {
  IO.mapRequired("Length", Section.Length);
  IO.mapRequired("Version", Section.Version);
  IO.mapRequired("UnitOffset", Section.UnitOffset);
  IO.mapRequired("UnitSize", Section.UnitSize);
  IO.mapRequired("Entries", Section.Entries);
}

void MappingTraits<DWARFYAML::Unit>::mapping(IO &IO, DWARFYAML::Unit &Unit) {
  IO.mapOptional("Format", Unit.Format, dwarf::DWARF32);
  IO.mapOptional("Length", Unit.Length);
  IO.mapRequired("Version", Unit.Version);
  if (Unit.Version >= 5)
    IO.mapRequired("UnitType", Unit.Type);
  IO.mapRequired("AbbrOffset", Unit.AbbrOffset);
  IO.mapRequired("AddrSize", Unit.AddrSize);
  IO.mapOptional("Entries", Unit.Entries);
}

void MappingTraits<DWARFYAML::Entry>::mapping(IO &IO, DWARFYAML::Entry &Entry) {
  IO.mapRequired("AbbrCode", Entry.AbbrCode);
  IO.mapOptional("Values", Entry.Values);
}

void MappingTraits<DWARFYAML::FormValue>::mapping(
    IO &IO, DWARFYAML::FormValue &FormValue) {
  IO.mapOptional("Value", FormValue.Value);
  if (!FormValue.CStr.empty() || !IO.outputting())
    IO.mapOptional("CStr", FormValue.CStr);
  if (!FormValue.BlockData.empty() || !IO.outputting())
    IO.mapOptional("BlockData", FormValue.BlockData);
}

void MappingTraits<DWARFYAML::File>::mapping(IO &IO, DWARFYAML::File &File) {
  IO.mapRequired("Name", File.Name);
  IO.mapRequired("DirIdx", File.DirIdx);
  IO.mapRequired("ModTime", File.ModTime);
  IO.mapRequired("Length", File.Length);
}

void MappingTraits<DWARFYAML::LineTableOpcode>::mapping(
    IO &IO, DWARFYAML::LineTableOpcode &LineTableOpcode) {
  IO.mapRequired("Opcode", LineTableOpcode.Opcode);
  if (LineTableOpcode.Opcode == dwarf::DW_LNS_extended_op) {
    IO.mapRequired("ExtLen", LineTableOpcode.ExtLen);
    IO.mapRequired("SubOpcode", LineTableOpcode.SubOpcode);
  }

  if (!LineTableOpcode.UnknownOpcodeData.empty() || !IO.outputting())
    IO.mapOptional("UnknownOpcodeData", LineTableOpcode.UnknownOpcodeData);
  if (!LineTableOpcode.UnknownOpcodeData.empty() || !IO.outputting())
    IO.mapOptional("StandardOpcodeData", LineTableOpcode.StandardOpcodeData);
  if (!LineTableOpcode.FileEntry.Name.empty() || !IO.outputting())
    IO.mapOptional("FileEntry", LineTableOpcode.FileEntry);
  if (LineTableOpcode.Opcode == dwarf::DW_LNS_advance_line || !IO.outputting())
    IO.mapOptional("SData", LineTableOpcode.SData);
  IO.mapOptional("Data", LineTableOpcode.Data);
}

void MappingTraits<DWARFYAML::LineTable>::mapping(
    IO &IO, DWARFYAML::LineTable &LineTable) {
  IO.mapOptional("Format", LineTable.Format, dwarf::DWARF32);
  IO.mapRequired("Length", LineTable.Length);
  IO.mapRequired("Version", LineTable.Version);
  IO.mapRequired("PrologueLength", LineTable.PrologueLength);
  IO.mapRequired("MinInstLength", LineTable.MinInstLength);
  if(LineTable.Version >= 4)
    IO.mapRequired("MaxOpsPerInst", LineTable.MaxOpsPerInst);
  IO.mapRequired("DefaultIsStmt", LineTable.DefaultIsStmt);
  IO.mapRequired("LineBase", LineTable.LineBase);
  IO.mapRequired("LineRange", LineTable.LineRange);
  IO.mapRequired("OpcodeBase", LineTable.OpcodeBase);
  IO.mapRequired("StandardOpcodeLengths", LineTable.StandardOpcodeLengths);
  IO.mapRequired("IncludeDirs", LineTable.IncludeDirs);
  IO.mapRequired("Files", LineTable.Files);
  IO.mapRequired("Opcodes", LineTable.Opcodes);
}

void MappingTraits<DWARFYAML::SegAddrPair>::mapping(
    IO &IO, DWARFYAML::SegAddrPair &SegAddrPair) {
  IO.mapOptional("Segment", SegAddrPair.Segment, 0);
  IO.mapOptional("Address", SegAddrPair.Address, 0);
}

void MappingTraits<DWARFYAML::AddrTableEntry>::mapping(
    IO &IO, DWARFYAML::AddrTableEntry &AddrTable) {
  IO.mapOptional("Format", AddrTable.Format, dwarf::DWARF32);
  IO.mapOptional("Length", AddrTable.Length);
  IO.mapRequired("Version", AddrTable.Version);
  IO.mapOptional("AddressSize", AddrTable.AddrSize);
  IO.mapOptional("SegmentSelectorSize", AddrTable.SegSelectorSize, 0);
  IO.mapOptional("Entries", AddrTable.SegAddrPairs);
}

void MappingTraits<DWARFYAML::StringOffsetsTable>::mapping(
    IO &IO, DWARFYAML::StringOffsetsTable &StrOffsetsTable) {
  IO.mapOptional("Format", StrOffsetsTable.Format, dwarf::DWARF32);
  IO.mapOptional("Length", StrOffsetsTable.Length);
  IO.mapOptional("Version", StrOffsetsTable.Version, 5);
  IO.mapOptional("Padding", StrOffsetsTable.Padding, 0);
  IO.mapOptional("Offsets", StrOffsetsTable.Offsets);
}

void MappingTraits<DWARFYAML::DWARFOperation>::mapping(
    IO &IO, DWARFYAML::DWARFOperation &DWARFOperation) {
  IO.mapRequired("Operator", DWARFOperation.Operator);
  IO.mapOptional("Values", DWARFOperation.Values);
}

void MappingTraits<DWARFYAML::RnglistEntry>::mapping(
    IO &IO, DWARFYAML::RnglistEntry &RnglistEntry) {
  IO.mapRequired("Operator", RnglistEntry.Operator);
  IO.mapOptional("Values", RnglistEntry.Values);
}

void MappingTraits<DWARFYAML::LoclistEntry>::mapping(
    IO &IO, DWARFYAML::LoclistEntry &LoclistEntry) {
  IO.mapRequired("Operator", LoclistEntry.Operator);
  IO.mapOptional("Values", LoclistEntry.Values);
  IO.mapOptional("DescriptionsLength", LoclistEntry.DescriptionsLength);
  IO.mapOptional("Descriptions", LoclistEntry.Descriptions);
}

template <typename EntryType>
void MappingTraits<DWARFYAML::ListEntries<EntryType>>::mapping(
    IO &IO, DWARFYAML::ListEntries<EntryType> &ListEntries) {
  IO.mapOptional("Entries", ListEntries.Entries);
  IO.mapOptional("Content", ListEntries.Content);
}

template <typename EntryType>
StringRef MappingTraits<DWARFYAML::ListEntries<EntryType>>::validate(
    IO &IO, DWARFYAML::ListEntries<EntryType> &ListEntries) {
  if (ListEntries.Entries && ListEntries.Content)
    return "Entries and Content can't be used together";
  return StringRef();
}

template <typename EntryType>
void MappingTraits<DWARFYAML::ListTable<EntryType>>::mapping(
    IO &IO, DWARFYAML::ListTable<EntryType> &ListTable) {
  IO.mapOptional("Format", ListTable.Format, dwarf::DWARF32);
  IO.mapOptional("Length", ListTable.Length);
  IO.mapOptional("Version", ListTable.Version, 5);
  IO.mapOptional("AddressSize", ListTable.AddrSize);
  IO.mapOptional("SegmentSelectorSize", ListTable.SegSelectorSize, 0);
  IO.mapOptional("OffsetEntryCount", ListTable.OffsetEntryCount);
  IO.mapOptional("Offsets", ListTable.Offsets);
  IO.mapOptional("Lists", ListTable.Lists);
}

void MappingTraits<DWARFYAML::InitialLength>::mapping(
    IO &IO, DWARFYAML::InitialLength &InitialLength) {
  IO.mapRequired("TotalLength", InitialLength.TotalLength);
  if (InitialLength.isDWARF64())
    IO.mapRequired("TotalLength64", InitialLength.TotalLength64);
}

} // end namespace yaml

} // end namespace llvm
