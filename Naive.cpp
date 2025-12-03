#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <vector>

using namespace std;

class SuffixTree {
  private:
    struct Node;

    struct Edge {
        int l, r;
        Node *child;
    };

    struct Node {
        map<char, Edge> next;
        int suffixIndex = -1;
    };

    string text;
    vector<unique_ptr<Node>> pool;
    Node *root = nullptr;

    Node *newNode(int suf = -1) {
        pool.push_back(make_unique<Node>());
        pool.back()->suffixIndex = suf;
        return pool.back().get();
    }

    void insertSuffix(int pos) {
        Node *cur = root;
        int i = pos;
        int n = (int)text.size();

        while (i < n) {
            char c = text[i];
            auto it = cur->next.find(c);

            if (it == cur->next.end()) {
                Node *leaf = newNode(pos);
                cur->next[c] = Edge{i, n - 1, leaf};
                return;
            }

            Edge oldEdge = it->second;
            int l = oldEdge.l, r = oldEdge.r;
            int k = 0;
            while ((l + k) <= r && (i + k) < n && text[l + k] == text[i + k])
                k++;

            if ((l + k) > r) {
                cur = oldEdge.child;
                i += k;
                continue;
            }

            Node *mid = newNode();
            it->second = Edge{l, l + k - 1, mid};
            mid->next[text[l + k]] = Edge{l + k, r, oldEdge.child};
            Node *leaf = newNode(pos);
            mid->next[text[i + k]] = Edge{i + k, n - 1, leaf};

            return;
        }
    }

    void printRec(const Node *node, const string &prefix, bool isLast) const {
        cout << prefix;

        if (node != root) {
            cout << (isLast ? "└─" : "├─");
        }

        if (node == root) {
            cout << "raiz\n";
        } else {
            if (node->suffixIndex != -1) {
                cout << "hoja (inicio = " << node->suffixIndex << ")\n";
            } else {
                cout << "nodo\n";
            }
        }

        int cnt = 0;
        int total = (int)node->next.size();

        for (const auto &p : node->next) {
            cnt++;
            const Edge &e = p.second;
            string edgeLabel;
            edgeLabel.reserve(e.r - e.l + 1);
            for (int j = e.l; j <= e.r; j++)
                edgeLabel.push_back(text[j]);

            bool childLast = (cnt == total);
            string nextPrefix = prefix + (node == root ? "" : (isLast ? "  " : "│ "));
            cout << nextPrefix << (childLast ? "└─" : "├─") << "arista: \"" << edgeLabel << "\"\n";
            printRec(e.child, nextPrefix + (childLast ? "  " : "│ "), true);
        }
    }

  public:
    void build(string s) {
        if (s.empty() || s.back() != '$')
            s.push_back('$');

        text = std::move(s);
        pool.clear();
        root = newNode();

        for (int i = 0; i < (int)text.size(); i++)
            insertSuffix(i);
    }
    void print() const { printRec(root, "", true); }
};

int main() {
    SuffixTree st;

    string s = "esta es la primera prueba del suffix tree";
    st.build(s);

    cout << "texto: " << s << "\n\n";
    st.print();

    return 0;
}
