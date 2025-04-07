#pragma once
#include <algorithm>
#include <utility>
#include <vector>

#include "ContainerAllocator.h"


template <typename T, typename Allocator>
class TArray
{
public:
    using SizeType = typename Allocator::SizeType;

private:
    std::vector<T, Allocator> ContainerPrivate;

public:
    // Iterator를 사용하기 위함
    auto begin() noexcept { return ContainerPrivate.begin(); }
    auto end() noexcept { return ContainerPrivate.end(); }
    auto begin() const noexcept { return ContainerPrivate.begin(); }
    auto end() const noexcept { return ContainerPrivate.end(); }
    auto rbegin() noexcept { return ContainerPrivate.rbegin(); }
    auto rend() noexcept { return ContainerPrivate.rend(); }
    auto rbegin() const noexcept { return ContainerPrivate.rbegin(); }
    auto rend() const noexcept { return ContainerPrivate.rend(); }

    T& operator[](SizeType Index);
    const T& operator[](SizeType Index) const;
	void operator+(const TArray& OtherArray);

public:
    TArray();
    ~TArray() = default;

    // 이니셜라이저 생성자
    TArray(std::initializer_list<T> InitList);

    // 복사 생성자
    TArray(const TArray& Other);

    // 이동 생성자
    TArray(TArray&& Other) noexcept;

    // 복사 할당 연산자
    TArray& operator=(const TArray& Other);

    // 이동 할당 연산자
    TArray& operator=(TArray&& Other) noexcept;

	/** Element를 Number개 만큼 초기화 합니다. */
    void Init(const T& Element, SizeType Number);
    SizeType Add(const T& Item);
    SizeType Add(T&& Item);
    SizeType AddUnique(const T& Item);

	template <typename... Args>
    SizeType Emplace(Args&&... Item);

    /** Array가 비어있는지 확인합니다. */
    bool IsEmpty() const;

	/** Array를 비웁니다 */
    void Empty();

	/** Item과 일치하는 모든 요소를 제거합니다. */
    SizeType Remove(const T& Item);

	/** 왼쪽부터 Item과 일치하는 요소를 1개 제거합니다. */
    bool RemoveSingle(const T& Item);

	/** 특정 위치에 있는 요소를 제거합니다. */
    void RemoveAt(SizeType Index);
	void RemoveAt(SizeType Index, SizeType Count);

	/** Predicate에 부합하는 모든 요소를 제거합니다. */
    template <typename Predicate>
        requires std::is_invocable_r_v<bool, Predicate, const T&>
    SizeType RemoveAll(const Predicate& Pred);

    T* GetData();
    const T* GetData() const;

    /**
     * Array에서 Item을 찾습니다.
     * @param Item 찾으려는 Item
     * @return Item의 인덱스, 찾을 수 없다면 -1
     */
    SizeType Find(const T& Item);
    bool Find(const T& Item, SizeType& Index);

    /** 요소가 존재하는지 확인합니다. */
    bool Contains(const T& Item) const;

    /** Array Size를 가져옵니다. */
    SizeType Num() const;

    /** Array의 Capacity를 가져옵니다. */
    SizeType Len() const;

	/** Array의 Size를 Number로 설정합니다. */
	void SetNum(SizeType Number);

	/** Array의 Capacity를 Number로 설정합니다. */
    void Reserve(SizeType Number);

    void Sort();
    template <typename Compare>
        requires std::is_invocable_r_v<bool, Compare, const T&, const T&>
    void Sort(const Compare& CompFn);

    bool IsValidIndex(uint32 ElementIndex) const {
        if (ElementIndex < 0 || ElementIndex >= Len()) return false;

        return true;
    }
    void Append(const TArray& OtherArray);
    void ShrinkToFit();
    /** 마지막 요소를 반환합니다. 비어 있으면 assert 발생 */
    T& Last();
    const T& Last() const;

    /** 마지막 요소를 제거하고 반환합니다. */
    bool Pop(T& OutItem);
    T Pop();

    /** 요소를 마지막에 추가합니다. */
    void Push(const T& Item);
    void Push(T&& Item);

    /** 마지막 요소를 참조로 반환합니다. */
    T& Top();
    const T& Top() const;

    /** 첫 요소를 제거하고 반환합니다. */
    T Dequeue();

    /** 첫 요소로 추가합니다 (Queue용) */
    void Enqueue(const T& Item);
    void Enqueue(T&& Item);

};


template <typename T, typename Allocator>
T& TArray<T, Allocator>::operator[](SizeType Index)
{
    return ContainerPrivate[Index];
}

template <typename T, typename Allocator>
const T& TArray<T, Allocator>::operator[](SizeType Index) const
{
    return ContainerPrivate[Index];
}

template <typename T, typename Allocator>
void TArray<T, Allocator>::operator+(const TArray& OtherArray)
{
	ContainerPrivate.insert(end(), OtherArray.begin(), OtherArray.end());
}

template <typename T, typename Allocator>
TArray<T, Allocator>::TArray()
    : ContainerPrivate()
{
}

template <typename T, typename Allocator>
TArray<T, Allocator>::TArray(std::initializer_list<T> InitList)
    : ContainerPrivate(InitList)
{
}

template <typename T, typename Allocator>
TArray<T, Allocator>::TArray(const TArray& Other)
    : ContainerPrivate(Other.ContainerPrivate)
{
}

template <typename T, typename Allocator>
TArray<T, Allocator>::TArray(TArray&& Other) noexcept
    : ContainerPrivate(std::move(Other.ContainerPrivate))
{
}

template <typename T, typename Allocator>
TArray<T, Allocator>& TArray<T, Allocator>::operator=(const TArray& Other)
{
    if (this != &Other)
    {
        ContainerPrivate = Other.ContainerPrivate;
    }
    return *this;
}

template <typename T, typename Allocator>
TArray<T, Allocator>& TArray<T, Allocator>::operator=(TArray&& Other) noexcept
{
    if (this != &Other)
    {
        ContainerPrivate = std::move(Other.ContainerPrivate);
    }
    return *this;
}

template <typename T, typename Allocator>
void TArray<T, Allocator>::Init(const T& Element, SizeType Number)
{
    ContainerPrivate.assign(Number, Element);
}

template <typename T, typename Allocator>
typename TArray<T, Allocator>::SizeType TArray<T, Allocator>::Add(const T& Item)
{
    return Emplace(Item);
}

template <typename T, typename Allocator>
typename TArray<T, Allocator>::SizeType TArray<T, Allocator>::Add(T&& Item)
{
    return Emplace(std::move(Item));
}

template <typename T, typename Allocator>
typename TArray<T, Allocator>::SizeType TArray<T, Allocator>::AddUnique(const T& Item)
{
    if (SizeType Index; Find(Item, Index))
    {
        return Index;
    }
    return Add(Item);
}

template <typename T, typename Allocator>
template <typename... Args>
typename TArray<T, Allocator>::SizeType TArray<T, Allocator>::Emplace(Args&&... Item)
{
    ContainerPrivate.emplace_back(std::forward<Args>(Item)...);
    return Num()-1;
}

template <typename T, typename Allocator>
bool TArray<T, Allocator>::IsEmpty() const
{
    return ContainerPrivate.empty();
}

template <typename T, typename Allocator>
void TArray<T, Allocator>::Empty()
{
    ContainerPrivate.clear();
}

template <typename T, typename Allocator>
typename TArray<T, Allocator>::SizeType TArray<T, Allocator>::Remove(const T& Item)
{
    auto oldSize = ContainerPrivate.size();
    ContainerPrivate.erase(std::remove(ContainerPrivate.begin(), ContainerPrivate.end(), Item), ContainerPrivate.end());
    return static_cast<SizeType>(oldSize - ContainerPrivate.size());
}

template <typename T, typename Allocator>
bool TArray<T, Allocator>::RemoveSingle(const T& Item)
{
    auto it = std::find(ContainerPrivate.begin(), ContainerPrivate.end(), Item);
    if (it != ContainerPrivate.end())
    {
        ContainerPrivate.erase(it);
        return true;
    }
    return false;
}

template <typename T, typename Allocator>
void TArray<T, Allocator>::RemoveAt(SizeType Index)
{
    if (Index >= 0 && static_cast<SizeType>(Index) < ContainerPrivate.size())
    {
        ContainerPrivate.erase(ContainerPrivate.begin() + Index);
    }
}
template <typename T, typename Allocator>
void TArray<T, Allocator>::RemoveAt(SizeType Index, SizeType Count)
{
    if (Index >= 0 && static_cast<SizeType>(Index + Count) <= ContainerPrivate.size())
    {
        ContainerPrivate.erase(ContainerPrivate.begin() + Index, ContainerPrivate.begin() + Index + Count);
    }
}
template <typename T, typename Allocator>
template <typename Predicate>
    requires std::is_invocable_r_v<bool, Predicate, const T&>
typename TArray<T, Allocator>::SizeType TArray<T, Allocator>::RemoveAll(const Predicate& Pred)
{
    auto oldSize = ContainerPrivate.size();
    ContainerPrivate.erase(std::remove_if(ContainerPrivate.begin(), ContainerPrivate.end(), Pred), ContainerPrivate.end());
    return static_cast<SizeType>(oldSize - ContainerPrivate.size());
}

template <typename T, typename Allocator>
T* TArray<T, Allocator>::GetData()
{
    return ContainerPrivate.data();
}

template <typename T, typename Allocator>
const T* TArray<T, Allocator>::GetData() const
{
    return ContainerPrivate.data();
}

template <typename T, typename Allocator>
typename TArray<T, Allocator>::SizeType TArray<T, Allocator>::Find(const T& Item)
{
    const auto it = std::find(ContainerPrivate.begin(), ContainerPrivate.end(), Item);
    return it != ContainerPrivate.end() ? std::distance(ContainerPrivate.begin(), it) : -1;
}

template <typename T, typename Allocator>
bool TArray<T, Allocator>::Find(const T& Item, SizeType& Index)
{
    Index = Find(Item);
    return (Index != -1);
}

template <typename T, typename Allocator>
bool TArray<T, Allocator>::Contains(const T& Item) const
{
    for (const T* Data = GetData(), *DataEnd = Data + Num(); Data != DataEnd; ++Data)
    {
        if (*Data == Item)
        {
            return true;
        }
    }
    return false;
}

template <typename T, typename Allocator>
typename TArray<T, Allocator>::SizeType TArray<T, Allocator>::Num() const
{
    return ContainerPrivate.size();
}

template <typename T, typename Allocator>
typename TArray<T, Allocator>::SizeType TArray<T, Allocator>::Len() const
{
    return ContainerPrivate.capacity();
}

template <typename T, typename Allocator>
void TArray<T, Allocator>::SetNum(SizeType Number)
{
	ContainerPrivate.resize(Number);
}

template <typename T, typename Allocator>
void TArray<T, Allocator>::Reserve(SizeType Number)
{
    ContainerPrivate.reserve(Number);
}

template <typename T, typename Allocator>
void TArray<T, Allocator>::Sort()
{
    std::sort(ContainerPrivate.begin(), ContainerPrivate.end());
}

template <typename T, typename Allocator>
template <typename Compare>
    requires std::is_invocable_r_v<bool, Compare, const T&, const T&>
void TArray<T, Allocator>::Sort(const Compare& CompFn)
{
    std::sort(ContainerPrivate.begin(), ContainerPrivate.end(), CompFn);
}

template <typename T, typename Allocator = FDefaultAllocator<T>> class TArray;
template <typename T, typename Allocator>
void TArray<T, Allocator>::Append(const TArray& OtherArray)
{
    ContainerPrivate.insert(ContainerPrivate.end(), OtherArray.begin(), OtherArray.end());
}
template <typename T, typename Allocator>
void TArray<T, Allocator>::ShrinkToFit()
{
    ContainerPrivate.shrink_to_fit();
}
template <typename T>
TArray<T> Slice(const TArray<T>& InArray, int32 StartIndex, int32 Count)
{
    TArray<T> Result;
    if (StartIndex < 0 || Count <= 0 || StartIndex >= InArray.Num())
        return Result;

    int32 MaxCount = std::min(Count, InArray.Num() - StartIndex);
    Result.Reserve(MaxCount);
    for (int32 i = 0; i < MaxCount; ++i)
    {
        Result.Add(InArray[StartIndex + i]);
    }
    return Result;
}