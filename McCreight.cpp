#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include <algorithm>
using namespace std;

class SuffixTree {
  public:
    struct Node {
        unordered_map<unsigned char, Node *> next;
        Node *link = nullptr;
        Node *parent = nullptr;
        int start, end;
        int suffixIndex = -1;

        Node(int s = -1, int e = -1, int suf = -1) : start(s), end(e), suffixIndex(suf) {}
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

    bool contains(string P) {
        Node *v = root;
        int i = 0;

        while (i < (int)P.size()) {
        auto it = v->next.find(P[i]);
        if (it == v->next.end())
            return false;

        Node *nxt = it->second;
        int edgeLen = nxt->len();
        int j = 0;

        while (j < edgeLen && i < (int)P.size()) {
            if (s[nxt->start + j] != P[i])
            return false;
            j++;
            i++;
        }

        v = nxt;
        }
        return true;
    }

    void DFS(Node *node, vector<int> &indices) {
        if (node->next.empty()) {
        indices.push_back(node->suffixIndex);
        return;
        }

        for (auto &child : node->next)
        DFS(child.second, indices);
    }

    vector<int> findAll(string P) {
        vector<int> indices;
        Node *v = root;
        int i = 0;

        while (i < (int)P.size()) {
        auto it = v->next.find(P[i]);
        if (it == v->next.end())
            return {};

        Node *nxt = it->second;
        int edgeLen = nxt->len();
        int j = 0;

        while (j < edgeLen && i < (int)P.size()) {
            if (s[nxt->start + j] != P[i])
            return {};
            j++;
            i++;
        }

        v = nxt;
        }

        DFS(v, indices);
        return indices;
    }

    int countAll(string P) { return (int)findAll(P).size(); }

    string pathLabel(Node *v) {
        string label = "";

        while (v != root) {
        label = s.substr(v->start, v->len()) + label;
        v = v->parent;
        }

        return label;
    }

    int stringDepth(Node *v) {
        int depth = 0;

        while (v != root) {
        depth += v->len();
        v = v->parent;
        }

        return depth;
    }

    Node *getNodeFromPattern(const string &P) {
        Node *v = root;
        int i = 0;

        while (i < (int)P.size()) {
        auto it = v->next.find(P[i]);
        if (it == v->next.end())
            return nullptr;

        Node *nxt = it->second;
        int edgeLen = nxt->len();
        int j = 0;

        while (j < edgeLen && i < (int)P.size()) {
            if (s[nxt->start + j] != P[i])
            return nullptr;
            j++;
            i++;
        }

        v = nxt;
        }

        return v;
    }

    void dfsSuffixArray(Node *v, vector<int> &SA) {
        if (v->next.empty()) {
        SA.push_back(v->suffixIndex);
        return;
        }

        vector<pair<unsigned char, Node *>> children;
        for (auto &kv : v->next)
        children.push_back(kv);

        sort(children.begin(), children.end(),
            [](const auto &a, const auto &b) { return a.first < b.first; });

        for (auto &child : children)
        dfsSuffixArray(child.second, SA);
    }

    vector<int> toSuffixArray() {
        vector<int> SA;
        dfsSuffixArray(root, SA);
        return SA;
    }

    void print() const { printRec(root, "", true); }

  private:
    Node *makeNode(int s, int e, Node *p = nullptr, int suf = -1) {
        pool.push_back(make_unique<Node>(s, e, suf));
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
                v->next[c] = makeNode(j, s.size() - 1, v,i);
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
            Node *mid = makeNode(w->start, k - 1, v,-1);
            v->next[c] = mid;

            w->start = k;
            w->parent = mid;
            mid->next[s[k]] = w;

            Node *leaf = makeNode(j, s.size() - 1, mid,i);
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
    
    cout << "------CASO BASE: banana------\n\n";
    
    string text = "banana";
    SuffixTree st_base(text);

    cout << "texto: " << text << "\n\n";
    st_base.print();

    cout << "\nMétodo Contains:";
    cout << "\n   ana: " << st_base.contains("ana");
    cout << "\n   begin: " << st_base.contains("begin");

    cout << "\nMétodo findAll:";
    cout << endl << "      na: ";
    for (auto i : st_base.findAll("na")) {
        cout << i << " ";
    }

    cout << "\nMétodo getNodeFromPattern:\n";
    SuffixTree::Node *v = st_base.getNodeFromPattern("ana");

    if (v) {
        cout << "   Nodo de 'ana' encontrado.\n";
        cout << "\nMétodo PathLabel:\n";
        cout << "   PathLabel del nodo: " << st_base.pathLabel(v) << endl;
        cout << "\nMétodo stringDepth:\n";
        cout << "   stringDepth del nodo: " << st_base.stringDepth(v) << endl;
    }

    cout << "\nMétodo toSuffixArray:";
    cout << endl << "      ";
    for (auto i : st_base.toSuffixArray()) {
        cout << i << " ";
    }

    cout << "\n\n------CASO EXTENSO: Antiguo Testamento------\n";
    
    
    SuffixTree st = txt_to_suffix_tree("Bible.txt", limit);

    cout << "Suffix tree construido\n";

    //     SuffixTree st1("aaaaaaaaaaaaaaa$");
    //     st1.print();
    return 0;
}
