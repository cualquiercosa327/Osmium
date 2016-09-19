// automatically generated by the FlatBuffers compiler, do not modify

#ifndef FLATBUFFERS_GENERATED_MESHFORMAT_H_
#define FLATBUFFERS_GENERATED_MESHFORMAT_H_

#include "../../../External/flatbuffers/flatbuffers.h"

struct Vec2;

struct Vec3;

struct Vertex;

struct BinaryMesh;

MANUALLY_ALIGNED_STRUCT(4) Vec2 FLATBUFFERS_FINAL_CLASS {
 private:
  float x_;
  float y_;

 public:
  Vec2() { memset(this, 0, sizeof(Vec2)); }
  Vec2(const Vec2 &_o) { memcpy(this, &_o, sizeof(Vec2)); }
  Vec2(float _x, float _y)
    : x_(flatbuffers::EndianScalar(_x)), y_(flatbuffers::EndianScalar(_y)) { }

  float x() const { return flatbuffers::EndianScalar(x_); }
  float y() const { return flatbuffers::EndianScalar(y_); }
};
STRUCT_END(Vec2, 8);

MANUALLY_ALIGNED_STRUCT(4) Vec3 FLATBUFFERS_FINAL_CLASS {
 private:
  float x_;
  float y_;
  float z_;

 public:
  Vec3() { memset(this, 0, sizeof(Vec3)); }
  Vec3(const Vec3 &_o) { memcpy(this, &_o, sizeof(Vec3)); }
  Vec3(float _x, float _y, float _z)
    : x_(flatbuffers::EndianScalar(_x)), y_(flatbuffers::EndianScalar(_y)), z_(flatbuffers::EndianScalar(_z)) { }

  float x() const { return flatbuffers::EndianScalar(x_); }
  float y() const { return flatbuffers::EndianScalar(y_); }
  float z() const { return flatbuffers::EndianScalar(z_); }
};
STRUCT_END(Vec3, 12);

struct Vertex FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  enum {
    VT_POSITION = 4,
    VT_NORMAL = 6,
    VT_TEXTURE = 8
  };
  const Vec3 *Position() const { return GetStruct<const Vec3 *>(VT_POSITION); }
  const Vec3 *Normal() const { return GetStruct<const Vec3 *>(VT_NORMAL); }
  const Vec2 *Texture() const { return GetStruct<const Vec2 *>(VT_TEXTURE); }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyField<Vec3>(verifier, VT_POSITION) &&
           VerifyField<Vec3>(verifier, VT_NORMAL) &&
           VerifyField<Vec2>(verifier, VT_TEXTURE) &&
           verifier.EndTable();
  }
};

struct VertexBuilder {
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_Position(const Vec3 *Position) { fbb_.AddStruct(Vertex::VT_POSITION, Position); }
  void add_Normal(const Vec3 *Normal) { fbb_.AddStruct(Vertex::VT_NORMAL, Normal); }
  void add_Texture(const Vec2 *Texture) { fbb_.AddStruct(Vertex::VT_TEXTURE, Texture); }
  VertexBuilder(flatbuffers::FlatBufferBuilder &_fbb) : fbb_(_fbb) { start_ = fbb_.StartTable(); }
  VertexBuilder &operator=(const VertexBuilder &);
  flatbuffers::Offset<Vertex> Finish() {
    auto o = flatbuffers::Offset<Vertex>(fbb_.EndTable(start_, 3));
    return o;
  }
};

inline flatbuffers::Offset<Vertex> CreateVertex(flatbuffers::FlatBufferBuilder &_fbb,
    const Vec3 *Position = 0,
    const Vec3 *Normal = 0,
    const Vec2 *Texture = 0) {
  VertexBuilder builder_(_fbb);
  builder_.add_Texture(Texture);
  builder_.add_Normal(Normal);
  builder_.add_Position(Position);
  return builder_.Finish();
}

struct BinaryMesh FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  enum {
    VT_INDEXCOUNT = 4,
    VT_VERTICES = 6,
    VT_INDICES = 8
  };
  uint32_t IndexCount() const { return GetField<uint32_t>(VT_INDEXCOUNT, 0); }
  const flatbuffers::Vector<flatbuffers::Offset<Vertex>> *Vertices() const { return GetPointer<const flatbuffers::Vector<flatbuffers::Offset<Vertex>> *>(VT_VERTICES); }
  const flatbuffers::Vector<uint32_t> *Indices() const { return GetPointer<const flatbuffers::Vector<uint32_t> *>(VT_INDICES); }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyField<uint32_t>(verifier, VT_INDEXCOUNT) &&
           VerifyField<flatbuffers::uoffset_t>(verifier, VT_VERTICES) &&
           verifier.Verify(Vertices()) &&
           verifier.VerifyVectorOfTables(Vertices()) &&
           VerifyField<flatbuffers::uoffset_t>(verifier, VT_INDICES) &&
           verifier.Verify(Indices()) &&
           verifier.EndTable();
  }
};

struct BinaryMeshBuilder {
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_IndexCount(uint32_t IndexCount) { fbb_.AddElement<uint32_t>(BinaryMesh::VT_INDEXCOUNT, IndexCount, 0); }
  void add_Vertices(flatbuffers::Offset<flatbuffers::Vector<flatbuffers::Offset<Vertex>>> Vertices) { fbb_.AddOffset(BinaryMesh::VT_VERTICES, Vertices); }
  void add_Indices(flatbuffers::Offset<flatbuffers::Vector<uint32_t>> Indices) { fbb_.AddOffset(BinaryMesh::VT_INDICES, Indices); }
  BinaryMeshBuilder(flatbuffers::FlatBufferBuilder &_fbb) : fbb_(_fbb) { start_ = fbb_.StartTable(); }
  BinaryMeshBuilder &operator=(const BinaryMeshBuilder &);
  flatbuffers::Offset<BinaryMesh> Finish() {
    auto o = flatbuffers::Offset<BinaryMesh>(fbb_.EndTable(start_, 3));
    return o;
  }
};

inline flatbuffers::Offset<BinaryMesh> CreateBinaryMesh(flatbuffers::FlatBufferBuilder &_fbb,
    uint32_t IndexCount = 0,
    flatbuffers::Offset<flatbuffers::Vector<flatbuffers::Offset<Vertex>>> Vertices = 0,
    flatbuffers::Offset<flatbuffers::Vector<uint32_t>> Indices = 0) {
  BinaryMeshBuilder builder_(_fbb);
  builder_.add_Indices(Indices);
  builder_.add_Vertices(Vertices);
  builder_.add_IndexCount(IndexCount);
  return builder_.Finish();
}

inline flatbuffers::Offset<BinaryMesh> CreateBinaryMeshDirect(flatbuffers::FlatBufferBuilder &_fbb,
    uint32_t IndexCount = 0,
    const std::vector<flatbuffers::Offset<Vertex>> *Vertices = nullptr,
    const std::vector<uint32_t> *Indices = nullptr) {
  return CreateBinaryMesh(_fbb, IndexCount, Vertices ? _fbb.CreateVector<flatbuffers::Offset<Vertex>>(*Vertices) : 0, Indices ? _fbb.CreateVector<uint32_t>(*Indices) : 0);
}

#endif  // FLATBUFFERS_GENERATED_MESHFORMAT_H_