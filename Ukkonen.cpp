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

    bool contains(string P){
        Node * v = root;
        int i = 0;

        while(i < (int)P.size()){
            auto it = v->next.find(P[i]);
            if(it == v->next.end()){
                return false;
            }

            Node * nxt = it->second;

            int edgeLen = nxt->len();
            int j = 0;

            while(j < edgeLen && i < (int)P.size()){
                if(s[nxt->start + j] != P[i]){
                    return false;
                }
                j++;
                i++;
            }

            v = nxt;
        }
        
        return true;
    }

    void DFS(Node * node, vector<int> &indices){
        if(node->next.empty()){
            indices.push_back(node->suffixIndex);
            return;
        }

        for(auto &child : node->next){
            DFS(child.second, indices);
        }
    }

    vector<int> findAll(string P){
        vector<int> indices;
        Node * v = root;
        int i = 0;
        
        while(i < (int)P.size()){
            auto it = v->next.find(P[i]);
            if(it == v->next.end()){
                return {};
            }

            Node * nxt = it->second;
            int edgeLen = nxt->len();
            int j = 0;
            
            while(j < edgeLen && i < (int)P.size()){
                if(s[nxt->start + j] != P[i]){
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

int main() {
    string text = "esta es la primera prueba del suffix tree";
    SuffixTree st(text);

    cout << "texto: " << text << "\n\n";
    st.print();

    cout << "\nMétodo Contains:";
    cout << "\n   esta: " << st.contains("esta");
    cout << "\n   estas: " << st.contains("estas");

    return 0;
}
