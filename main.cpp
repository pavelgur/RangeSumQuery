#include <cassert>
#include <numeric>
#include <vector>
#include <unordered_map>


template<class T, class TOp = std::plus<>>
class TRsq {
public:
    TRsq(const std::vector<T>& data)
        : N(data.size())
    {
        if (N) {
            Tree.resize(4 * N);
            Build(data, 1, 0, N - 1);
        }
    }

    T Get(const size_t leftBound, const size_t rightBound) {
        FlushChanges();

        return Get(1, 0, N - 1, leftBound, rightBound);
    }

    void Modify(const size_t pos, T val) {
        if (pos >= N) {
            return;
        }

        Updates[pos] = std::move(val);
    }

private:
    void Build(const std::vector<T>& data, const size_t node, const size_t leftBound, const size_t rightBound) {
        if (leftBound == rightBound) {
            Tree[node] = data[leftBound];
        } else {
            const auto mid = leftBound + (rightBound - leftBound) / 2;
            const auto nn = 2 * node;

            Build(data, nn, leftBound, mid);
            Build(data, nn + 1, mid + 1, rightBound);

            Tree[node] = TOp()(Tree[nn], Tree[nn + 1]);
        }
    }

    T Get(const size_t node, const size_t treeLeftBound, const size_t treeRightBound, const size_t leftBound, const size_t rightBound) const {
        if (leftBound > rightBound) {
            return T();
        }

        if (leftBound == treeLeftBound && rightBound == treeRightBound) {
            return Tree[node];
        }

        const auto mid = treeLeftBound + (treeRightBound - treeLeftBound) / 2;
        const auto nn = 2 * node;

        return TOp()(
                    Get(nn, treeLeftBound, mid, leftBound, std::min(rightBound,mid)),
                    Get(nn + 1, mid + 1, treeRightBound, std::max(leftBound, mid+1), rightBound)
                    );
    }

    void FlushChanges() {
        if (Updates.empty()) {
            return;
        }

        for (auto& pair : Updates) {
            Update(1, 0, N - 1, pair.first, std::move(pair.second));
        }

        Updates.clear();
    }

    void Update(const size_t node, const size_t leftBound, const size_t rightBound, const size_t pos, T&& val) {
        if (leftBound == rightBound) {
            Tree[node] = std::move(val);
            return;
        }

        const auto mid = (leftBound + rightBound) / 2;
        const auto nn = 2 * node;

        if (pos <= mid) {
            Update(nn, leftBound, mid, pos, std::move(val));
        } else {
            Update(nn + 1, mid + 1, rightBound, pos, std::move(val));
        }

        Tree[node] = TOp()(Tree[nn], Tree[nn + 1]);
    }

private:
    const size_t N;
    std::vector<T> Tree;
    std::unordered_map<size_t, T> Updates;
};

template<class T>
struct TMin {
    const T& operator()(const T& a, const T& b) const noexcept {
        return std::min(a, b);
    }
};

template<class T>
struct TMax {
    const T& operator()(const T& a, const T& b) const noexcept {
        return std::max(a, b);
    }
};

int main() {
    using namespace std;

    vector<int> data(99);
    srand(777);
    for (auto i = 0u; i < data.size(); ++i) {
        data[i] = -100 + (rand() % 200);
    }

//    using TOp = TMin<int>;
//    using TOp = TMax<int>;
    using TOp = std::plus<>;
    TRsq<int, TOp> rsq(data);

    for (auto i = 0u; i + 1 < data.size(); ++i) {
        for (auto j = i + 1; j < data.size(); ++j) {

            for (auto k = 0u; k < rand() % 10; ++k) {
                const size_t modIdx = i + (rand() % (j - i));
                assert(modIdx < data.size());

                auto& v = data[modIdx] = -100 + (rand() % 200);

                rsq.Modify(modIdx, v);
            }

            const auto trueSum = std::accumulate(data.begin() + i, data.begin() + j + 1, 0, TOp());
            const auto gotSum = rsq.Get(i, j);
            assert(trueSum == gotSum);
        }
    }

    return EXIT_SUCCESS;
}
