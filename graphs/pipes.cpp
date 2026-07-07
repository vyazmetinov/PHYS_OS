#include <iostream>
#include <vector>
#include <queue>
#include <algorithm>

using namespace std;

struct Edge {
    int to;
    int capacity;
    int rev;
};

void add_edge(vector<vector<Edge>>& graph, int from, int to, int capacity) {
    Edge forward{to, capacity, (int) graph[to].size()};
    Edge backward{from, 0, (int) (graph[from].size())};
    graph[from].push_back(forward);
    graph[to].push_back(backward);
}

int bfs(vector<vector<Edge>>& graph, int s, int t, vector<int>& parent_vertex, vector<int>& parent_edge) {
    int n = graph.size();
    fill(parent_vertex.begin(), parent_vertex.end(), -1);
    fill(parent_edge.begin(), parent_edge.end(), -1);

    queue<int> q;
    q.push(s);
    parent_vertex[s] = s;

    while (!q.empty()) {
        int v = q.front();
        q.pop();

        for (int i = 0; i < (int)(graph[v].size()); ++i) {
            Edge& e = graph[v][i];
            if (parent_vertex[e.to] == -1 && e.capacity > 0) {
                parent_vertex[e.to] = v;
                parent_edge[e.to] = i;
                if (e.to == t) {
                    int flow = 1000000000;
                    int cur = t;
                    while (cur != s) {
                        int prev = parent_vertex[cur];
                        int edge_index = parent_edge[cur];
                        flow = min(flow, graph[prev][edge_index].capacity);
                        cur = prev;
                    }
                    return flow;
                }
                q.push(e.to);
            }
        }
    }

    return 0;
}

int max_flow(vector<vector<Edge>>& graph, int s, int t) {
    int n = graph.size();
    vector<int> parent_vertex(n);
    vector<int> parent_edge(n);
    int flow = 0;

    while (true) {
        int pushed = bfs(graph, s, t, parent_vertex, parent_edge);
        if (pushed == 0) {
            break;
        }

        flow += pushed;
        int cur = t;
        while (cur != s) {
            int prev = parent_vertex[cur];
            int edge_index = parent_edge[cur];
            int rev_index = graph[prev][edge_index].rev;

            graph[prev][edge_index].capacity -= pushed;
            graph[cur][rev_index].capacity += pushed;

            cur = prev;
        }
    }

    return flow;
}

int main() {
    int n, m;
    cin >> n >> m;

    vector<vector<Edge>> graph(n);

    for (int i = 0; i < m; ++i) {
        int u, v, c;
        cin >> u >> v >> c;
        add_edge(graph, u, v, c);
    }

    int s, t;
    cin >> s >> t;

    cout << max_flow(graph, s, t);
    return 0;
}