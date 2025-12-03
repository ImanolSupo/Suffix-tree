#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
using namespace std;

class SuffixTree {
  public:
    struct Node {
        unordered_map<unsigned char, Node *> next;
        Node *link = nullptr;
        Node *parent = nullptr;
        int start, end;

        Node(int s = -1, int e = -1) : start(s), end(e) {}
        int len() const { return end - start + 1; }
    };

    string s;
    Node *root;
    vector<unique_ptr<Node>> pool;

    SuffixTree(string text) {
        if (text.empty() || text.back() != '$')
            text.push_back('$');
        s = text;
        root = makeNode(-1, -1);
        build();
    }

    void print() const { printRec(root, "", true); }

  private:
    Node *makeNode(int s, int e, Node *p = nullptr) {
        pool.push_back(make_unique<Node>(s, e));
        Node *v = pool.back().get();
        v->parent = p;
        return v;
    }

    void build() {
        Node *curHead = root; // h en el paper
        Node *prevInternal = nullptr;

        for (int i = 0; i < (int)s.size(); i++) {
            tie(curHead, prevInternal) = insertSuffix(i, curHead, prevInternal);
        }
    }

    pair<Node *, Node *> insertSuffix(int i, Node *head, Node *prevInternal) {
        Node *v = head;

        // seguir suffix link si existe
        if (v != root)
            v = (v->link ? v->link : root);

        int j = i;
        while (true) {
            unsigned char c = s[j];

            // si no hay arista, crear hoja y salir
            if (!v->next.count(c)) {
                v->next[c] = makeNode(j, s.size() - 1, v);
                if (prevInternal)
                    prevInternal->link = v;
                return {v, nullptr};
            }

            Node *w = v->next[c];
            int k = w->start;

            // caminar por la arista
            while (k <= w->end && j < (int)s.size() && s[k] == s[j]) {
                k++;
                j++;
            }

            // si la arista coincide totalmente continuar bajando
            if (k > w->end) {
                v = w;
                continue;
            }

            // mismatch -> split
            Node *mid = makeNode(w->start, k - 1, v);
            v->next[c] = mid;

            w->start = k;
            w->parent = mid;
            mid->next[s[k]] = w;

            Node *leaf = makeNode(j, s.size() - 1, mid);
            mid->next[s[j]] = leaf;

            if (prevInternal)
                prevInternal->link = mid;
            return {mid, mid};
        }
    }

    void printRec(Node *v, const string &pref, bool last) const {
        if (v == root) {
            cout << "raiz\n";
        } else {
            cout << pref << (last ? "└─" : "├─") << "nodo\n";
        }
        vector<pair<unsigned char, Node *>> kids;
        kids.reserve(v->next.size());
        for (auto &kv : v->next)
            kids.push_back(kv);

        for (int i = 0; i < (int)kids.size(); i++) {
            bool childLast = (i == (int)kids.size() - 1);
            Node *to = kids[i].second;

            // imprimir arista como Ukkonen
            string nextPref = (v == root ? "" : pref + (last ? "  " : "│ "));
            cout << nextPref << (childLast ? "└─" : "├─") << "arista \"" << s.substr(to->start, to->len()) << "\"\n";

            printRec(to, nextPref + (childLast ? "  " : "│ "), true);
        }
    }
};

SuffixTree txt_to_suffix_tree(const string &filename, long long limit) {
    ifstream in(filename);
    if (!in.is_open()) {
        cerr << "Error: no se pudo abrir el archivo\n";
        exit(1);
    }

    string text;
    text.assign((istreambuf_iterator<char>(in)), istreambuf_iterator<char>());

    long long n = text.size();
    string used = (limit < n ? text.substr(0, (size_t)limit) : text);

    if (used.empty() || used.back() != '$')
        used.push_back('$');

    return SuffixTree(used);
}

int main() {
    long long limit = 70000; // limite de caracteres, 4 322 868 caracteres como maximo
    SuffixTree st = txt_to_suffix_tree("Bible.txt", limit);

    cout << "Suffix tree construido\n";

    //     SuffixTree st1("aaaaaaaaaaaaaaa$");
    //     st1.print();
    return 0;
}
