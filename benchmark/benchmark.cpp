#include <chrono>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
using namespace std;

// NAIVE

class NaiveSuffixTree {
    struct Node {
        map<char, Node *> next;
        int l, r;
        int suf;
        Node(int L, int R, int S = -1) : l(L), r(R), suf(S) {}
    };

    string s;
    vector<unique_ptr<Node>> pool;
    Node *root;

    Node *newNode(int l, int r, int suf = -1) {
        pool.push_back(make_unique<Node>(l, r, suf));
        return pool.back().get();
    }

    void insertSuffix(int pos) {
        Node *cur = root;
        int i = pos, n = s.size();
        while (i < n) {
            char c = s[i];
            if (!cur->next.count(c)) {
                cur->next[c] = newNode(i, n - 1, pos);
                return;
            }
            Node *e = cur->next[c];
            int k = e->l;
            while (k <= e->r && i < n && s[k] == s[i])
                k++, i++;
            if (k > e->r) {
                cur = e;
                continue;
            }
            Node *mid = newNode(e->l, k - 1);
            cur->next[c] = mid;
            e->l = k;
            mid->next[s[k]] = e;
            mid->next[s[i]] = newNode(i, n - 1, pos);
            return;
        }
    }

  public:
    NaiveSuffixTree(const string &txt) {
        s = txt;
        root = newNode(-1, -1);
        for (int i = 0; i < (int)s.size(); i++)
            insertSuffix(i);
    }
};

// MCCREIGHT

class McCreightSuffixTree {
    struct Node {
        unordered_map<unsigned char, Node *> next;
        Node *link = nullptr;
        Node *parent = nullptr;
        int l, r;
        Node(int L = -1, int R = -1) : l(L), r(R) {}
        int len() const { return r - l + 1; }
    };

    string s;
    vector<unique_ptr<Node>> pool;
    Node *root;

    Node *newNode(int l, int r, Node *p = nullptr) {
        pool.push_back(make_unique<Node>(l, r));
        pool.back()->parent = p;
        return pool.back().get();
    }

    pair<Node *, Node *> insertSuffix(int i, Node *head, Node *prev) {
        Node *v = head;
        if (v != root)
            v = v->link ? v->link : root;

        int j = i;
        while (true) {
            unsigned char c = s[j];
            if (!v->next.count(c)) {
                v->next[c] = newNode(j, s.size() - 1, v);
                if (prev)
                    prev->link = v;
                return {v, nullptr};
            }

            Node *w = v->next[c];
            int k = w->l;
            while (k <= w->r && j < (int)s.size() && s[k] == s[j])
                k++, j++;

            if (k > w->r) {
                v = w;
                continue;
            }

            Node *mid = newNode(w->l, k - 1, v);
            v->next[c] = mid;

            w->l = k;
            w->parent = mid;
            mid->next[s[k]] = w;
            mid->next[s[j]] = newNode(j, s.size() - 1, mid);

            if (prev)
                prev->link = mid;
            return {mid, mid};
        }
    }

  public:
    McCreightSuffixTree(string txt) {
        if (txt.empty() || txt.back() != '$')
            txt.push_back('$');
        s = txt;
        root = newNode(-1, -1);
        Node *cur = root;
        Node *prev = nullptr;
        for (int i = 0; i < (int)s.size(); i++)
            tie(cur, prev) = insertSuffix(i, cur, prev);
    }
};

// UKKONEN

class UkkonenSuffixTree {
    struct Node {
        unordered_map<unsigned char, Node *> next;
        Node *link = nullptr;
        int start;
        int *end;
        Node(int s, int *e) : start(s), end(e) {}
        int len() const { return *end - start + 1; }
    };

    string s;
    vector<unique_ptr<Node>> pool;
    vector<unique_ptr<int>> ends;
    Node *root, *active;
    int activeEdge = -1, activeLen = 0, rem = 0;
    Node *last = nullptr;
    int leafEnd = -1, rootEnd = -1;

    Node *newNode(int s, int *e) {
        pool.push_back(make_unique<Node>(s, e));
        return pool.back().get();
    }

    int *newEnd(int v) {
        ends.push_back(make_unique<int>(v));
        return ends.back().get();
    }

    bool walk(Node *v) {
        int L = v->len();
        if (activeLen >= L) {
            activeEdge += L;
            activeLen -= L;
            active = v;
            return true;
        }
        return false;
    }

    void extend(int pos) {
        leafEnd = pos;
        rem++;
        last = nullptr;

        while (rem > 0) {
            if (activeLen == 0)
                activeEdge = pos;
            unsigned char c = s[activeEdge];

            if (!active->next.count(c)) {
                active->next[c] = newNode(pos, &leafEnd);
                if (last)
                    last->link = active, last = nullptr;
            } else {
                Node *nxt = active->next[c];
                if (walk(nxt))
                    continue;

                if (s[nxt->start + activeLen] == s[pos]) {
                    if (last && active != root)
                        last->link = active;
                    activeLen++;
                    break;
                }

                int *splitEnd = newEnd(nxt->start + activeLen - 1);
                Node *split = newNode(nxt->start, splitEnd);
                active->next[c] = split;

                nxt->start += activeLen;
                split->next[s[nxt->start]] = nxt;

                split->next[s[pos]] = newNode(pos, &leafEnd);
                if (last)
                    last->link = split;
                last = split;
            }

            rem--;

            if (active == root && activeLen > 0) {
                activeLen--;
                activeEdge = pos - rem + 1;
            } else if (active != root) {
                active = active->link ? active->link : root;
            }
        }
    }

  public:
    UkkonenSuffixTree(string txt) {
        if (txt.empty() || txt.back() != '$')
            txt.push_back('$');
        s = txt;
        root = newNode(-1, &rootEnd);
        root->link = root;
        active = root;
        for (int i = 0; i < (int)s.size(); i++)
            extend(i);
    }
};

string load_prefix(const string &fname, int limit) {
    ifstream in(fname);
    if (!in.is_open()) {
        cerr << "Error archivo\n";
        exit(1);
    }
    string text((istreambuf_iterator<char>(in)), {});
    if ((int)text.size() > limit)
        text.resize(limit);
    if (text.empty() || text.back() != '$')
        text.push_back('$');
    return text;
}

long long now_ms() { return chrono::duration_cast<chrono::milliseconds>(chrono::high_resolution_clock::now().time_since_epoch()).count(); }

// BENCHMARK

struct Result {
    int n;
    long long t1, t2, t3;
};

Result bench(int n) {
    string txt = load_prefix("Bible.txt", n);
    Result R;
    R.n = n;

    auto t0 = now_ms();
    NaiveSuffixTree a(txt);
    R.t1 = now_ms() - t0;
    t0 = now_ms();
    McCreightSuffixTree b(txt);
    R.t2 = now_ms() - t0;
    t0 = now_ms();
    UkkonenSuffixTree c(txt);
    R.t3 = now_ms() - t0;

    return R;
}

int main() {
    vector<int> T = {100, 2500, 5000, 7500, 10000, 15000, 20000, 25000, 30000, 35000, 40000, 45000, 50000};

    vector<Result> R;
    cout << "Ejecutando benchmark...\n";

    for (int n : T)
        R.push_back(bench(n));

    ofstream out("benchmark_results.txt");
    out << "n,naive,mccreight,ukkonen\n";
    for (auto &x : R)
        out << x.n << "," << x.t1 << "," << x.t2 << "," << x.t3 << "\n";

    cout << "Listo. Guardado en benchmark_results.txt\n";
    return 0;
}
