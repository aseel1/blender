/* SPDX-FileCopyrightText: 2023 Blender Authors
 *
 * SPDX-License-Identifier: Apache-2.0 */

#include "BLI_exception_safety_test_utils.hh"
#include "BLI_vector_set.hh"

#include "testing/testing.h"

#include "BLI_strict_flags.h" /* IWYU pragma: keep. Keep last. */

namespace blender::tests {

TEST(vector_set, DefaultConstructor)
{
  VectorSet<int> set;
  EXPECT_EQ(set.size(), 0);
  EXPECT_TRUE(set.is_empty());
}

TEST(vector_set, InitializerListConstructor_WithoutDuplicates)
{
  VectorSet<int> set = {1, 4, 5};
  EXPECT_EQ(set.size(), 3);
  EXPECT_EQ(set[0], 1);
  EXPECT_EQ(set[1], 4);
  EXPECT_EQ(set[2], 5);
}

TEST(vector_set, InitializerListConstructor_WithDuplicates)
{
  VectorSet<int> set = {1, 3, 3, 2, 1, 5};
  EXPECT_EQ(set.size(), 4);
  EXPECT_EQ(set[0], 1);
  EXPECT_EQ(set[1], 3);
  EXPECT_EQ(set[2], 2);
  EXPECT_EQ(set[3], 5);
}

TEST(vector_set, Copy)
{
  VectorSet<int> set1 = {1, 2, 3};
  VectorSet<int> set2 = set1;
  EXPECT_EQ(set1.size(), 3);
  EXPECT_EQ(set2.size(), 3);
  EXPECT_EQ(set1.index_of(2), 1);
  EXPECT_EQ(set2.index_of(2), 1);
}

TEST(vector_set, CopyAssignment)
{
  VectorSet<int> set1 = {1, 2, 3};
  VectorSet<int> set2 = {};
  set2 = set1;
  EXPECT_EQ(set1.size(), 3);
  EXPECT_EQ(set2.size(), 3);
  EXPECT_EQ(set1.index_of(2), 1);
  EXPECT_EQ(set2.index_of(2), 1);
}

TEST(vector_set, Move)
{
  VectorSet<int> set1 = {1, 2, 3};
  VectorSet<int> set2 = std::move(set1);
  EXPECT_EQ(set1.size(), 0); /* NOLINT: bugprone-use-after-move */
  EXPECT_EQ(set2.size(), 3);
}

TEST(vector_set, MoveAssignment)
{
  VectorSet<int> set1 = {1, 2, 3};
  VectorSet<int> set2 = {};
  set2 = std::move(set1);
  EXPECT_EQ(set1.size(), 0); /* NOLINT: bugprone-use-after-move */
  EXPECT_EQ(set2.size(), 3);
}

TEST(vector_set, AddNewIncreasesSize)
{
  VectorSet<int> set;
  EXPECT_TRUE(set.is_empty());
  EXPECT_EQ(set.size(), 0);
  set.add(5);
  EXPECT_FALSE(set.is_empty());
  EXPECT_EQ(set.size(), 1);
}

TEST(vector_set, AddExistingDoesNotIncreaseSize)
{
  VectorSet<int> set;
  EXPECT_EQ(set.size(), 0);
  EXPECT_TRUE(set.add(5));
  EXPECT_EQ(set.size(), 1);
  EXPECT_FALSE(set.add(5));
  EXPECT_EQ(set.size(), 1);
}

TEST(vector_set, Index)
{
  VectorSet<int> set = {3, 6, 4};
  EXPECT_EQ(set.index_of(6), 1);
  EXPECT_EQ(set.index_of(3), 0);
  EXPECT_EQ(set.index_of(4), 2);
}

TEST(vector_set, IndexTry)
{
  VectorSet<int> set = {3, 6, 4};
  EXPECT_EQ(set.index_of_try(5), -1);
  EXPECT_EQ(set.index_of_try(3), 0);
  EXPECT_EQ(set.index_of_try(6), 1);
  EXPECT_EQ(set.index_of_try(2), -1);
}

TEST(vector_set, RemoveContained)
{
  VectorSet<int> set = {4, 5, 6, 7};
  EXPECT_EQ(set.size(), 4);
  set.remove_contained(5);
  EXPECT_EQ(set.size(), 3);
  EXPECT_EQ(set[0], 4);
  EXPECT_EQ(set[1], 7);
  EXPECT_EQ(set[2], 6);
  set.remove_contained(6);
  EXPECT_EQ(set.size(), 2);
  EXPECT_EQ(set[0], 4);
  EXPECT_EQ(set[1], 7);
  set.remove_contained(4);
  EXPECT_EQ(set.size(), 1);
  EXPECT_EQ(set[0], 7);
  set.remove_contained(7);
  EXPECT_EQ(set.size(), 0);
}

TEST(vector_set, RemoveIf)
{
  VectorSet<int64_t> set;
  for (const int64_t i : IndexRange(100)) {
    set.add(i * i);
  }
  const int64_t removed = set.remove_if([](const int64_t key) { return key % 2 == 0; });
  EXPECT_EQ(set.size() + removed, 100);
  for (const int64_t i : IndexRange(100)) {
    EXPECT_EQ(set.contains(i * i), i % 2 == 1);
  }
}

TEST(vector_set, AddMultipleTimes)
{
  VectorSet<int> set;
  for (int i = 0; i < 100; i++) {
    EXPECT_FALSE(set.contains(i * 13));
    set.add(i * 12);
    set.add(i * 13);
    EXPECT_TRUE(set.contains(i * 13));
  }
}

TEST(vector_set, UniquePtrValue)
{
  VectorSet<std::unique_ptr<int>> set;
  set.add_new(std::make_unique<int>());
  set.add(std::make_unique<int>());
  set.index_of_try(std::make_unique<int>());
  std::unique_ptr<int> value = set.pop();
  UNUSED_VARS(value);
}

TEST(vector_set, Remove)
{
  VectorSet<int> set;
  EXPECT_TRUE(set.add(5));
  EXPECT_TRUE(set.contains(5));
  EXPECT_FALSE(set.remove(6));
  EXPECT_TRUE(set.contains(5));
  EXPECT_TRUE(set.remove(5));
  EXPECT_FALSE(set.contains(5));
  EXPECT_FALSE(set.remove(5));
  EXPECT_FALSE(set.contains(5));
}

TEST(vector_set, SpanConstructorExceptions)
{
  std::array<ExceptionThrower, 5> array = {1, 2, 3, 4, 5};
  array[3].throw_during_copy = true;
  Span<ExceptionThrower> span = array;

  EXPECT_ANY_THROW({ VectorSet<ExceptionThrower> set(span); });
}

TEST(vector_set, CopyConstructorExceptions)
{
  VectorSet<ExceptionThrower> set = {1, 2, 3, 4, 5};
  set[3].throw_during_copy = true;

  EXPECT_ANY_THROW({ VectorSet<ExceptionThrower> set_copy(set); });
}

TEST(vector_set, MoveConstructorExceptions)
{
  VectorSet<ExceptionThrower> set = {1, 2, 3, 4, 5};
  set[3].throw_during_copy = true;
  set[3].throw_during_move = true;
  /* Currently never throws on move, because values are separately allocated. */
  VectorSet<ExceptionThrower> set_moved(std::move(set));
  EXPECT_EQ(set.size(), 0); /* NOLINT: bugprone-use-after-move */
  set.add_multiple({4, 5, 6, 7, 8});
  EXPECT_EQ(set.size(), 5);
}

TEST(vector_set, AddNewExceptions)
{
  VectorSet<ExceptionThrower> set;
  ExceptionThrower value;
  value.throw_during_copy = true;
  EXPECT_ANY_THROW({ set.add_new(value); });
  EXPECT_EQ(set.size(), 0);
  EXPECT_ANY_THROW({ set.add_new(value); });
  EXPECT_EQ(set.size(), 0);
}

TEST(vector_set, AddExceptions)
{
  VectorSet<ExceptionThrower> set;
  ExceptionThrower value;
  value.throw_during_copy = true;
  EXPECT_ANY_THROW({ set.add(value); });
  EXPECT_EQ(set.size(), 0);
  EXPECT_ANY_THROW({ set.add(value); });
  EXPECT_EQ(set.size(), 0);
}

TEST(vector_set, ReserveExceptions)
{
  VectorSet<ExceptionThrower> set;
  set.add_multiple({1, 2, 3, 4, 5});
  set[2].throw_during_move = true;
  EXPECT_ANY_THROW({ set.reserve(100); });
}

TEST(vector_set, PopExceptions)
{
  VectorSet<ExceptionThrower> set = {1, 2, 3};
  set.as_span().last().throw_during_move = true;
  EXPECT_EQ(set.size(), 3);
  EXPECT_ANY_THROW({ set.pop(); }); /* NOLINT: bugprone-throw-keyword-missing */
  EXPECT_EQ(set.size(), 3);
  set.add(10);
  EXPECT_EQ(set.size(), 4);
}

TEST(vector_set, IndexOfOrAdd)
{
  VectorSet<int> set;
  EXPECT_EQ(set.index_of_or_add(3), 0);
  EXPECT_EQ(set.index_of_or_add(3), 0);
  EXPECT_EQ(set.index_of_or_add(2), 1);
  EXPECT_EQ(set.index_of_or_add(0), 2);
  EXPECT_EQ(set.index_of_or_add(2), 1);
  EXPECT_EQ(set.index_of_or_add(3), 0);
  EXPECT_EQ(set.index_of_or_add(5), 3);
  EXPECT_EQ(set.index_of_or_add(8), 4);
  EXPECT_EQ(set.index_of_or_add(5), 3);
}

TEST(vector_set, Clear)
{
  VectorSet<int> set = {4, 6, 2, 4};
  EXPECT_EQ(set.size(), 3);
  set.clear();
  EXPECT_EQ(set.size(), 0);
  set.add_multiple({4, 1, 6, 8, 3, 6, 9, 3});
  EXPECT_EQ(set.size(), 6);
  set.clear();
  EXPECT_EQ(set.size(), 0);
}

TEST(vector_set, LookupKey)
{
  VectorSet<std::string> set;
  set.add("a");
  set.add("b");
  set.add("c");
  EXPECT_EQ(set.lookup_key("a"), "a");
  EXPECT_EQ(set.lookup_key_as("c"), "c");
  EXPECT_EQ(set.lookup_key_ptr_as("d"), nullptr);
  EXPECT_EQ(set.lookup_key_ptr_as("b")->size(), 1);
  EXPECT_EQ(set.lookup_key_ptr("a"), set.lookup_key_ptr_as("a"));
  EXPECT_EQ(set.lookup_key_default("d", "default"), "default");
  EXPECT_EQ(set.lookup_key_default("c", "default"), "c");
}

TEST(vector_set, GrowWhenEmpty)
{
  /* Tests that the internal keys array is freed correctly when growing an empty set. */
  VectorSet<int> set;
  set.add(4);
  set.remove(4);
  EXPECT_TRUE(set.is_empty());
  set.reserve(100);
}

TEST(vector_set, ExtractVector)
{
  VectorSet<int> set;
  set.add_multiple({5, 2, 7, 4, 8, 5, 4, 5});
  EXPECT_EQ(set.size(), 5);
  const int *data_ptr = set.data();

  Vector<int> vec = set.extract_vector();
  EXPECT_EQ(vec.size(), 5);
  EXPECT_EQ(vec.data(), data_ptr);
}

TEST(vector_set, ExtractVectorEmpty)
{
  VectorSet<int> set;
  Vector<int> vec = set.extract_vector();
  EXPECT_TRUE(vec.is_empty());
}

TEST(vector_set, CustomIDVectorSet)
{
  struct ThingWithID {
    int a;
    std::string b;
    int c;
  };
  struct ThingGetter {
    StringRef operator()(const ThingWithID &value) const
    {
      return value.b;
    }
  };
  CustomIDVectorSet<ThingWithID, ThingGetter> set;
  set.add_new(ThingWithID{0, "test", 54});
  EXPECT_TRUE(set.contains_as("test"));
  set.add_new(ThingWithID{4333, "other", 2});
  EXPECT_EQ(set.size(), 2);
  set.add(ThingWithID{3333, "test", 27});
  EXPECT_EQ(set.size(), 2);
}

namespace {
struct KeyWithData {
  int key;
  std::string data;

  KeyWithData(int key, std::string data) : key(key), data(data) {}
  KeyWithData(const StringRef data) : key(0), data(data) {}

  uint64_t hash() const
  {
    return uint64_t(this->key);
  }

  friend bool operator==(const KeyWithData &a, const KeyWithData &b)
  {
    return a.key == b.key;
  }
};
}  // namespace

TEST(vector_set, AddOverwrite)
{
  VectorSet<KeyWithData> set;
  EXPECT_TRUE(set.add_overwrite(KeyWithData{1, "a"}));
  EXPECT_EQ(set.size(), 1);
  EXPECT_EQ(set[0].data, "a");
  EXPECT_FALSE(set.add(KeyWithData{1, "b"}));
  EXPECT_EQ(set.size(), 1);
  EXPECT_EQ(set[0].data, "a");
  EXPECT_EQ(set.lookup_key(KeyWithData{1, "_"}).data, "a");
  EXPECT_FALSE(set.add_overwrite(KeyWithData{1, "c"}));
  EXPECT_EQ(set.size(), 1);
  EXPECT_EQ(set[0].data, "c");
  EXPECT_EQ(set.lookup_key(KeyWithData{1, "_"}).data, "c");

  const KeyWithData key{2, "d"};
  EXPECT_TRUE(set.add_overwrite(key));
  EXPECT_EQ(set.size(), 2);
  EXPECT_EQ(set[0].data, "c");
  EXPECT_EQ(set[1].data, "d");
  EXPECT_EQ(set.lookup_key(key).data, "d");
}

TEST(vector_set, LookupDefault)
{
  VectorSet<KeyWithData> set;
  EXPECT_TRUE(set.add(KeyWithData{1, "b"}));
  EXPECT_TRUE(set.add(KeyWithData{423, "s"}));
  EXPECT_TRUE(set.add(KeyWithData{2, "t"}));
  KeyWithData default_key{1, "default"};
  EXPECT_EQ(set.lookup_key_default(KeyWithData{1221, "3"}, default_key), default_key);
  KeyWithData other_s{0, "testing"};
  EXPECT_EQ(set.lookup_key_default(KeyWithData{2398745, "fffff"}, StringRef("testing")), other_s);
  EXPECT_EQ(set.lookup_key_default(KeyWithData{1, "s"}, StringRef("testing")), set[0]);
  EXPECT_EQ(set.lookup_key_default(KeyWithData{2, "t"}, {1, "default"}), set[2]);
}

}  // namespace blender::tests
