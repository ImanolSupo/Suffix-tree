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
        int start = -1;
        int suffixIndex = -1;
        int *end = nullptr;
        Node(int s, int *e) : start(s), end(e) {}
        int len() const { return *end - start + 1; }
    };

    string s;

    vector<unique_ptr<Node>> pool;
    vector<unique_ptr<int>> ends;
    Node *root = nullptr;
    Node *active = nullptr;
    int activeEdge = -1;
    int activeLen = 0;
    int rem = 0;
    Node *lastInternal = nullptr;
    int leafEndVal = -1;
    int rootEndVal = -1;

  public:
    explicit SuffixTree(string text) {
        if (text.empty() || text.back() != '$')
            text.push_back('$');
        s = std::move(text);
        build();
    }

    void build() {
        pool.clear();
        ends.clear();

        rootEndVal = -1;
        leafEndVal = -1;

        root = newNode(-1, &rootEndVal);
        root->link = root;
        active = root;
        activeEdge = -1;
        activeLen = 0;
        rem = 0;
        lastInternal = nullptr;

        for (int i = 0; i < (int)s.size(); i++)
            extend(i);
    }

    void print() const { printRec(root, "", true); }

    bool contains(string P) {
        Node *v = root;
        int i = 0;

        while (i < (int)P.size()) {
            auto it = v->next.find(P[i]);
            if (it == v->next.end()) {
                return false;
            }

            Node *nxt = it->second;

            int edgeLen = nxt->len();
            int j = 0;

            while (j < edgeLen && i < (int)P.size()) {
                if (s[nxt->start + j] != P[i]) {
                    return false;
                }
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

        for (auto &child : node->next) {
            DFS(child.second, indices);
        }
    }

    vector<int> findAll(string P) {
        vector<int> indices;
        Node *v = root;
        int i = 0;

        while (i < (int)P.size()) {
            auto it = v->next.find(P[i]);
            if (it == v->next.end()) {
                return {};
            }

            Node *nxt = it->second;
            int edgeLen = nxt->len();
            int j = 0;

            while (j < edgeLen && i < (int)P.size()) {
                if (s[nxt->start + j] != P[i]) {
                    return {};
                }
                j++;
                i++;
            }
            v = nxt;
        }
        DFS(v, indices);
        return indices;
    }
    
    int countAll(string P){
        return findAll(P).size();
    }

    Node* findParentRec(Node* cur, Node* target){
        for(auto &kv : cur->next){
            if(kv.second == target)
                return cur;

            Node* res = findParentRec(kv.second, target);
            if(res) return res;
        }
        return nullptr;
    }

    Node* findParent(Node* target){
        return findParentRec(root, target);
    }

    string pathLabel(Node * v){
        string label = "";

        while(v != root){
            // obtener la etiqueta de la arista hacia v
            string edge;
            for(int i = v->start; i <= *(v->end); i++){
                edge.push_back(s[i]);
            }

            label = edge + label;

            v = findParent(v);
        }

        return label;
    }

    int stringDepth(Node * v){
        int depth = 0;
        while(v != root){
            // obtener la etiqueta de la arista hacia v
            string edge;
            for(int i = v->start; i <= *(v->end); i++){
                edge.push_back(s[i]);
            }

            depth += (int)edge.size();

            v = findParent(v);
        }
        return depth;
    }

    Node* getNodeFromPattern(const string& P){
        Node* v = root;
        int i = 0;

        while(i < (int)P.size()){
            auto it = v->next.find(P[i]);
            if(it == v->next.end())
                return nullptr;

            Node* nxt = it->second;
            int edgeLen = nxt->len();
            int j = 0;

            while(j < edgeLen && i < (int)P.size()){
                if(s[nxt->start + j] != P[i])
                    return nullptr;
                j++;
                i++;
            }

            v = nxt;
        }

        return v;
    }


    void dfsSuffixArray(Node* v, vector<int>& SA) {
        if (v->next.empty()) {
            SA.push_back(v->suffixIndex);
            return;
        }

        vector<pair<unsigned char, Node*>> children;
        for (auto& kv : v->next) {
            children.push_back(kv);
        }

        sort(children.begin(), children.end(),
            [](const auto& a, const auto& b) {
                return a.first < b.first;
            });

        for (auto& child : children) {
            dfsSuffixArray(child.second, SA);
        }
    }


    vector<int> toSuffixArray() {
        vector<int> SA;
        dfsSuffixArray(root, SA);
        return SA;
    }

  private:
    Node *newNode(int start, int *endPtr) {
        pool.push_back(make_unique<Node>(start, endPtr));
        return pool.back().get();
    }

    int *newEnd(int v) {
        ends.push_back(make_unique<int>(v));
        return ends.back().get();
    }

    bool walkDown(Node *v) {
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
        leafEndVal = pos;
        rem++;
        lastInternal = nullptr;

        while (rem > 0) {
            if (activeLen == 0)
                activeEdge = pos;

            unsigned char a = (unsigned char)s[activeEdge];

            auto it = active->next.find(a);
            if (it == active->next.end()) {
                Node *leaf = newNode(pos, &leafEndVal);
                leaf->suffixIndex = pos - rem + 1;
                active->next[a] = leaf;

                if (lastInternal != nullptr) {
                    lastInternal->link = active;
                    lastInternal = nullptr;
                }
            } else {
                Node *nxt = it->second;

                if (walkDown(nxt))
                    continue;

                unsigned char b = (unsigned char)s[nxt->start + activeLen];
                unsigned char c = (unsigned char)s[pos];

                if (b == c) {
                    if (lastInternal != nullptr && active != root) {
                        lastInternal->link = active;
                        lastInternal = nullptr;
                    }
                    activeLen++;
                    break;
                }

                int *splitEnd = newEnd(nxt->start + activeLen - 1);
                Node *split = newNode(nxt->start, splitEnd);
                split->link = root;
                active->next[a] = split;
                nxt->start += activeLen;
                split->next[(unsigned char)s[nxt->start]] = nxt;

                split->next[c] = newNode(pos, &leafEndVal);
                split->next[c]->suffixIndex = pos - rem + 1;

                if (lastInternal != nullptr)
                    lastInternal->link = split;
                lastInternal = split;
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

    void printRec(Node *v, const string &pref, bool last) const {
        if (v == root) {
            cout << "raiz\n";
        } else {
            cout << pref << (last ? "└─" : "├─") << "nodo\n";
        }

        vector<pair<unsigned char, Node *>> kids;
        kids.reserve(v->next.size());
        for (auto &kv : v->next)
            kids.push_back({kv.first, kv.second});

        for (int i = 0; i < (int)kids.size(); i++) {
            bool childLast = (i == (int)kids.size() - 1);
            Node *to = kids[i].second;
            string nextPref = (v == root ? "" : pref + (last ? "  " : "│ "));
            cout << nextPref << (childLast ? "└─" : "├─") << "arista \"" << label(to->start, *(to->end)) << "\"\n";

            printRec(to, nextPref + (childLast ? "  " : "│ "), true);
        }
    }

    string label(int l, int r) const {
        int maxShow = 60;
        string out;
        for (int i = l; i <= r && (int)out.size() < maxShow; i++)
            out.push_back(s[i]);
        if (r - l + 1 > maxShow)
            out += "...";
        return out;
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
    long long limit = 70000; // limite de caracteres
    SuffixTree st = txt_to_suffix_tree("Bible.txt", limit);
    cout << "Suffix tree construido \n";

    cout << "\nMétodo Contains:";
    cout << "\n   And God saw the light, that it was good: " << st.contains("And God saw the light, that it was good");
    cout << "\n   Come, let us make our father drink wine: " << st.contains("Come, let us make our father drink wine");

    cout << "\nMétodo FindAll:";
    cout << "\n   Eve: ";
    
    for(int f : st.findAll("Eve")){
        cout << f << " ";
    };

    cout << "\n";

    cout << "\nMétodo CountAll:";
    cout << "\n   Adam: " << st.countAll("Adam") << "\n";

    return 0;
}
