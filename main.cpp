#include <iostream>
#include <vector>
#include <queue>
#include <map>
#include <set>
#include <unordered_set>
#include <fstream>
#include <sstream>
#include <limits>
#include <algorithm>
#include <cctype>
#include <windows.h>

using namespace std;

const int INF = 1e9;

struct Edge {
    string to;
    int weight;
};

struct Step {
    int iteration;
    string current;
    vector<string> visited; // in-time visit order
    map<string, pair<int, string>> state; // node -> (dist, prev)
    map<string, int> updatedAt; // node -> iteration when value was last updated
};

map<string, vector<Edge>> graph;
vector<string> nodes;
vector<Step> processSteps;

void addBidirectionalEdge(const string& from, const string& to, int weight){
    graph[from].push_back({to, weight});
    if(from != to){
        graph[to].push_back({from, weight});
    }
}

void addDirectedEdge(const string& from, const string& to, int weight){
    graph[from].push_back({to, weight});
}

bool directedEdgeExists(const string& from, const string& to){
    auto it = graph.find(from);
    if(it == graph.end()) return false;
    for(const auto& edge : it->second){
        if(edge.to == to) return true;
    }
    return false;
}

void clearInputBuffer(){
    cin.clear();
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
}

bool readStrictIntLine(const string& prompt, int& value){
    cout << prompt;
    string line;
    if(!getline(cin, line)){
        return false;
    }

    stringstream ss(line);
    long long parsed;
    string extra;
    if(!(ss >> parsed) || (ss >> extra)){
        return false;
    }

    if(parsed < numeric_limits<int>::min() || parsed > numeric_limits<int>::max()){
        return false;
    }

    value = static_cast<int>(parsed);
    return true;
}

bool readPositiveIntLine(const string& prompt, int& value){
    cout << prompt;
    string line;
    if(!getline(cin, line)){
        return false;
    }

    stringstream ss(line);
    long long parsed;
    string extra;
    if(!(ss >> parsed) || (ss >> extra) || parsed <= 0){
        return false;
    }

    if(parsed > numeric_limits<int>::max()){
        return false;
    }

    value = static_cast<int>(parsed);
    return true;
}

bool readSingleTokenLine(const string& prompt, string& value){
    cout << prompt;
    string line;
    if(!getline(cin, line)){
        return false;
    }

    stringstream ss(line);
    string token, extra;
    if(!(ss >> token) || (ss >> extra)){
        return false;
    }

    value = token;
    return true;
}

bool readEdgeLine(const string& prompt, string& source, string& destination, int& weight){
    cout << prompt;
    string line;
    if(!getline(cin, line)){
        return false;
    }

    stringstream ss(line);
    string rawSource, rawDestination;
    int parsedWeight;
    string extra;
    if(!(ss >> rawSource >> rawDestination >> parsedWeight) || (ss >> extra)){
        return false;
    }

    source = rawSource;
    destination = rawDestination;
    weight = parsedWeight;
    return true;
}

bool readMenuChoice(int& mode){
    string line;
    if(!getline(cin, line)){
        return false;
    }

    stringstream ss(line);
    long long parsed;
    string extra;
    if(!(ss >> parsed) || (ss >> extra)){
        return false;
    }

    if(parsed < 0 || parsed > 2 || parsed > numeric_limits<int>::max()){
        return false;
    }

    mode = static_cast<int>(parsed);
    return true;
}

bool containsAlphabet(const string& value){
    for(unsigned char ch : value){
        if(isalpha(ch)) return true;
    }
    return false;
}

void setupPredefinedGraph(){
    nodes = {"V1","V2","V3","V4","V5","V6","V7",
             "V8","V9","V10","V11","V12","V13","V14"};

    graph.clear();

    addBidirectionalEdge("V1", "V2", 4);
    addBidirectionalEdge("V1", "V3", 6);
    addBidirectionalEdge("V1", "V4", 10);
    addBidirectionalEdge("V2", "V4", 3);

    addBidirectionalEdge("V2", "V5", 5);
    addBidirectionalEdge("V2", "V6", 12);
    addBidirectionalEdge("V4", "V5", 4);

    addBidirectionalEdge("V3", "V5", 2);
    addBidirectionalEdge("V3", "V7", 7);

    addBidirectionalEdge("V4", "V7", 3);
    addBidirectionalEdge("V4", "V8", 8);

    addBidirectionalEdge("V5", "V9", 6);
    addBidirectionalEdge("V5", "V10", 4);
    addBidirectionalEdge("V5", "V6", 2);

    addBidirectionalEdge("V6", "V10", 2);
    addBidirectionalEdge("V6", "V11", 7);
    addBidirectionalEdge("V6", "V7", 2);

    addBidirectionalEdge("V7", "V11", 3);
    addBidirectionalEdge("V7", "V12", 5);
    addBidirectionalEdge("V7", "V8", 4);

    addBidirectionalEdge("V8", "V12", 4);
    addBidirectionalEdge("V8", "V11", 3);

    addBidirectionalEdge("V9", "V13", 5);
    addBidirectionalEdge("V10", "V13", 2);
    addBidirectionalEdge("V10", "V11", 2);
    addBidirectionalEdge("V9", "V10", 3);

    addBidirectionalEdge("V11", "V14", 6);
    addBidirectionalEdge("V12", "V14", 3);
    addBidirectionalEdge("V11", "V12", 2);
    addBidirectionalEdge("V12", "V13", 4);
}

vector<string> reconstructPath(map<string,string>& previous, string destination){
    vector<string> path;
    if(previous.find(destination)==previous.end() && find(nodes.begin(), nodes.end(), destination)==nodes.end()){
        return path; // empty
    }

    // if destination unreachable, previous may be "" and dist INF; reconstruct will still push destination only if reachable
    string cur = destination;
    while(cur != ""){
        path.push_back(cur);
        cur = previous[cur];
    }
    reverse(path.begin(), path.end());
    return path;
}

pair<map<string,int>, map<string,string>> dijkstra(string source){
    processSteps.clear();

    map<string,int> dist;
    map<string,string> previous;
    unordered_set<string> visitedSet;
    vector<string> visitOrder;

    for(auto &node : nodes){
        dist[node] = INF;
        previous[node] = "";
    }

    if(dist.find(source)==dist.end()){
        return {dist, previous};
    }

    dist[source] = 0;

    map<string,int> lastUpdatedAt;
    for(auto &node : nodes){
        lastUpdatedAt[node] = -1;
    }
    lastUpdatedAt[source] = 0;

    priority_queue< pair<int,string>, vector<pair<int,string>>, greater<pair<int,string>> > pq;
    pq.push({0, source});

    int iteration = 1;

    while(!pq.empty()){
        string current = pq.top().second;
        pq.pop();

        if(visitedSet.count(current)) continue;

        visitedSet.insert(current);
        visitOrder.push_back(current);

        // relax neighbors
        for(auto &edge : graph[current]){
            if(dist[current] == INF) continue;
            int newDist = dist[current] + edge.weight;
            if(newDist < dist[edge.to]){
                dist[edge.to] = newDist;
                previous[edge.to] = current;
                lastUpdatedAt[edge.to] = iteration;
                pq.push({newDist, edge.to});
                cout << "Update " << edge.to << " = " << dist[edge.to] << " via " << current << endl;
            }
        }

        // record state AFTER all relaxations so the table shows cumulative distances
        Step step;
        step.iteration = iteration++;
        step.current = current;
        step.visited = visitOrder;

        map<string,int> distSnapshot = dist;
        map<string,string> previousSnapshot = previous;

        for(auto &node : nodes){
            step.state[node] = { distSnapshot[node], previousSnapshot[node] };
            step.updatedAt[node] = lastUpdatedAt[node];
        }

        processSteps.push_back(step);
    }

    return {dist, previous};
}

string vectorToJSArray(const vector<string>& arr){
    stringstream ss;
    ss << "[";
    for(size_t i=0;i<arr.size();++i){
        ss << "'" << arr[i] << "'";
        if(i+1!=arr.size()) ss << ",";
    }
    ss << "]";
    return ss.str();
}

string toLowerCase(string s){
    transform(s.begin(), s.end(), s.begin(), ::tolower);
    return s;
}

string toUpperCase(string s){
    transform(s.begin(), s.end(), s.begin(), ::toupper);
    return s;
}

void generateHTML(const vector<string>& shortestPath, int totalWeight, const string& source, const string& destination, bool manualPositions, const map<string, pair<int,int>>& positions){
    ofstream html("index.html");

    html << "<!DOCTYPE html>\n<html>\n<head>\n<meta charset=\"UTF-8\">\n";
    html << "<script src=\"https://unpkg.com/cytoscape/dist/cytoscape.min.js\"></script>\n";
    html << "<title>Dijkstra Game Visualization</title>\n<style>\n";
    html << "body{font-family:Arial,Helvetica,sans-serif;background:#ffffff;color:#111827;margin:0;padding:0;line-height:1.45;}\n";
    html << ".page{width:100%;max-width:none;margin:0 auto;padding:22px 24px 40px;box-sizing:border-box;}\n";
    html << ".top-section{width:100%;} \n";
    html << ".graph-section{width:100%;height:1000px;display:block;position:relative;overflow:visible;margin-top:20px;}\n";
    html << ".table-section{width:100%;margin-top:10px;}\n";
    html << ".guide{max-width:1100px;margin:12px auto 18px;padding:16px 18px;background:#f8fafc;border:1px solid #e5e7eb;border-radius:16px;box-shadow:0 6px 18px rgba(15,23,42,0.05);text-align:left;}\n";
    html << ".guide h3{margin:0 0 10px;font-size:18px;color:#111827;}\n";
    html << ".guide p{margin:6px 0;color:#374151;}\n";
    html << ".guide .nodes{font-weight:700;color:#111827;}\n";
    html << ".graph-container{width:100%;max-width:none;overflow:visible;position:relative;}\n";
    html << "#cy{width:100%;height:1000px;min-height:1000px;background:#ffffff;border:2px solid #d1d5db;border-radius:20px;position:relative;display:block;}\n";
    html << ".table-wrap{width:100%;overflow-x:auto;overflow-y:hidden;}\n";
    html << "table{width:100%;min-width:1100px;margin:20px auto 0;border-collapse:collapse;table-layout:auto;background:#ffffff;color:#111827;font-size:13px;}\n";
    html << "th, td{border:1px solid #ddd;padding:6px 8px;text-align:center;vertical-align:middle;}\n";
    html << "thead th{background:#f5f5f5;font-weight:bold;}\n";
    html << "thead tr:first-child th{position:sticky;top:0;z-index:2;}\n";
    html << "tbody tr:nth-child(odd) td{background:#ffffff;}\n";
    html << "tbody tr:nth-child(even) td{background:#f9fafb;}\n";
    html << "tbody td{vertical-align:middle;}\n";
    html << "th.node-col, td.node-cell{min-width:55px;}\n";
    html << "td.node-cell{white-space:nowrap;line-height:1.15;}\n";
    html << "td.current-cell{background:#e8f5e9 !important;border-color:#b7dfb9 !important;font-weight:700;}\n";
    html << "td.updated-cell{background:#fff7cc !important;border-color:#f2d27a !important;font-weight:700;}\n";
    html << "td.minimum-cell{background:#dbeafe !important;border:1px solid #60a5fa !important;font-weight:700;color:#111827;}\n";
    html << "td.final-path-cell{background:#bfdbfe !important;border:1px solid #3b82f6 !important;font-weight:700;color:#111827;}\n";
    html << ".iter-tag{font-size:9px;vertical-align:sub;color:#6b7280;margin-left:2px;line-height:1;}\n";
    html << ".set-block{display:inline-block;white-space:normal;line-height:1.25;text-align:center;}\n";
    html << ".result{margin:12px auto 0;padding:14px 18px;font-size:16px;color:#111827;background:#f8fafc;border:1px solid #e5e7eb;border-radius:14px;display:inline-block;max-width:100%;}\n";
    html << ".legend{display:flex;justify-content:center;gap:14px;flex-wrap:wrap;margin:10px 0 8px;}\n";
    html << ".legend-item{display:inline-flex;align-items:center;gap:8px;padding:7px 12px;border:1px solid #e5e7eb;border-radius:999px;background:#f9fafb;color:#111827;font-size:14px;}\n";
    html << ".legend-dot{width:12px;height:12px;border-radius:999px;display:inline-block;border:1px solid #334155;}\n";
    html << ".dot-start{background:#22c55e;}\n";
    html << ".dot-end{background:#ef4444;}\n";
    html << ".dot-node{background:#4FC3F7;}\n";
    html << "h1,h2{color:#111827;margin-bottom:10px;}\n";
    html << "</style>\n</head>\n<body>\n";

    html << "<div class='page'>\n";
    html << "<div class='top-section'>\n";
    html << "<h1>🤖 Mekorama Inspired — Dijkstra Visualization</h1>\n";
    html << "<div class='legend'>\n";
    html << "<span class='legend-item'><span class='legend-dot dot-start'></span>Start Node</span>\n";
    html << "<span class='legend-item'><span class='legend-dot dot-end'></span>Goal Node</span>\n";
    html << "<span class='legend-item'><span class='legend-dot dot-node'></span>Normal Node</span>\n";
    html << "</div>\n";
    html << "<div class='guide'>\n";
    html << "<h3>Cara Input Node</h3>\n";
    html << "<p>Masukkan node dengan format yang benar. Jalur akan dicari otomatis menggunakan algoritma Dijkstra.</p>\n";
    html << "<p>Node awal: <strong>V1</strong> | Node tujuan: <strong>V14</strong></p>\n";
    html << "<p class='nodes'>Node tersedia: V1, V2, V3, V4, V5, V6, V7, V8, V9, V10, V11, V12, V13, V14</p>\n";
    html << "</div>\n";
    html << "<div class='result' id='resultArea'>\n";
    if(!shortestPath.empty()){
        html << "<h2>Shortest Path</h2>\n<p>";
        for(size_t i=0;i<shortestPath.size();++i){
            html << shortestPath[i];
            if(i+1!=shortestPath.size()) html << " → ";
        }
        html << "</p>\n<p>Total Bobot: " << totalWeight << "</p>\n";
    } else {
        html << "<h2>Graph Preview</h2><p>Graph ditampilkan. Masukkan node awal & tujuan di console.</p>\n";
    }
    html << "</div>\n";
    html << "</div>\n";

    html << "<div class='graph-section'>\n<div class='graph-container'>\n<div id=\"cy\"></div>\n</div>\n</div>\n";

    html << "<div class='table-section'>\n<h2>Tabel Proses Dijkstra</h2>\n<div class='table-wrap'>\n<table>\n<thead>\n<tr><th>Iteration</th><th>Unvisited(Q)</th><th>Visited(S)</th><th>Current</th><th colspan='" << nodes.size() << "'>Node : Min = (dist[node], prev[node])</th></tr>\n<tr><th></th><th></th><th></th><th></th>";
    for(auto &n : nodes) html << "<th class='node-col'>" << toUpperCase(n) << "</th>";
    html << "</tr>\n</thead>\n<tbody>\n";

    auto formatNodeSet = [&](const vector<string>& list){
        stringstream ss;
        ss << "{";
        for(size_t i=0;i<list.size();++i){
            ss << toUpperCase(list[i]);
            if(i+1!=list.size()) ss << ",";
        }
        ss << "}";
        return ss.str();
    };

    unordered_set<string> finalPathNodes(shortestPath.begin(), shortestPath.end());

    for(size_t stepIndex=0; stepIndex<processSteps.size(); ++stepIndex){
        auto &step = processSteps[stepIndex];
        vector<string> unvisited;
        unordered_set<string> visitedLookup(step.visited.begin(), step.visited.end());
        for(const auto& node : nodes){
            if(!visitedLookup.count(node)) unvisited.push_back(node);
        }

        html << "<tr>";
        html << "<td>" << step.iteration << "</td>";
        html << "<td><div class='set-block'>" << formatNodeSet(unvisited) << "</div></td>";
        html << "<td><div class='set-block'>" << formatNodeSet(step.visited) << "</div></td>";
        html << "<td class='current-cell'>" << toUpperCase(step.current) << "</td>";

        for(auto &n : nodes){
            auto s = step.state.at(n);
            string distStr = s.first >= INF ? string("∞") : to_string(s.first);
            string prevStr = s.second.empty() ? string("-") : toUpperCase(s.second);
            int updatedAt = step.updatedAt.at(n);
            bool changedNow = updatedAt == step.iteration;
            bool isCurrentMinimum = (n == step.current);
            html << "<td class='node-cell";
            if(stepIndex == processSteps.size()-1 && finalPathNodes.count(n)){
                html << " final-path-cell";
            } else if(isCurrentMinimum){
                html << " minimum-cell";
            } else if(changedNow){
                html << " updated-cell";
            }
            html << "'>";
            html << "(" << distStr << "," << prevStr << ")";
            if(updatedAt >= 0){
                html << "<span class='iter-tag'>" << updatedAt << "</span>";
            }
            html << "</td>";
        }

        html << "</tr>\n";
    }

    html << "</tbody>\n</table>\n</div>\n</div>\n";

    // JavaScript: elements, styles, layout, path highlighting, animation
    html << "<script>\n";

    // shortestPath array
    html << "let shortestPath = " << vectorToJSArray(shortestPath) << ";\n";
    html << "let isManualPositions = " << (manualPositions ? "true" : "false") << ";\n";

    // nodes data
    html << "let elements = [\n";

    for(auto &n : nodes){
        html << "{ data: { id: '" << n << "', label: '" << toUpperCase(n) << "' }";
        if(manualPositions){
            auto it = positions.find(n);
            if(it!=positions.end()){
                html << ", position: { x: " << it->second.first << ", y: " << it->second.second << " }";
            }
        }
        html << " },\n";
    }

    // edges
    for(auto &p : graph){
        for(auto &e : p.second){
            html << "{ data: { source: '" << p.first << "', target: '" << e.to << "', label: '" << e.weight << "' } },\n";
        }
    }

    html << "];\n";
    if(manualPositions){
        html << "console.log(elements);\n";
    }

    // cytoscape init
    html << "var cy = cytoscape({ container: document.getElementById('cy'), pixelRatio: 1, wheelSensitivity: 0.2, ";
    html << "autoungrabify: true, ";
    html << "userPanningEnabled: true, userZoomingEnabled: true, boxSelectionEnabled: false, elements: elements, style: [ { selector:'node', style:{ 'label':'data(label)','background-color':'#4FC3F7','color':'#111827','font-size':16,'font-weight':'bold','text-valign':'center','text-halign':'center','width':65,'height':65,'border-width':3,'border-color':'#1f2937' } }, ";
    if(manualPositions){
        html << "{ selector:'edge', style:{ 'label':'data(label)','curve-style':'bezier','control-point-step-size':55,'width':4,'line-color':'#6b7280','target-arrow-color':'#6b7280','target-arrow-shape':'triangle','font-size':14,'font-weight':'bold','color':'#111827','text-background-color':'#ffffff','text-background-opacity':1,'text-background-padding':4,'text-border-color':'#d1d5db','text-border-width':1,'text-rotation':'autorotate' } }, ";
    } else {
        html << "{ selector:'edge', style:{ 'label':'data(label)','curve-style':'bezier','control-point-step-size':70,'width':3,'line-color':'#6b7280','target-arrow-color':'#6b7280','arrow-scale':1.2,'target-arrow-shape':'triangle','font-size':14,'font-weight':'bold','color':'#111827','text-background-color':'#ffffff','text-background-opacity':1,'text-background-padding':3,'text-border-color':'#d1d5db','text-border-width':1,'text-rotation':'autorotate','text-margin-y':-8 } }, ";
    }
    html << "{ selector:'.highlighted', style:{ 'line-color':'#f59e0b','target-arrow-color':'#f59e0b','width':7 } }, { selector:'.start', style:{ 'background-color':'#22c55e' } }, { selector:'.end', style:{ 'background-color':'#ef4444' } } ], minZoom:0.8, maxZoom:2.5, ";

    if(manualPositions){
        html << "layout:{ name:'preset', fit:false, padding:0 }";
    } else {
        html << "layout:{ name:'cose', animate:false, randomize:true, fit:true, padding:140, avoidOverlap:true, nodeDimensionsIncludeLabels:true, idealEdgeLength:220, nodeRepulsion:30000, edgeElasticity:120, gravity:0.08, numIter:1800, initialTemp:200 }";
    }

    html << " });\n";
    html << "cy.userPanningEnabled(true);\n";
    html << "cy.panningEnabled(true);\n";
    html << "cy.on('layoutstop', function(){ cy.nodes().lock(); });\n";
    html << "cy.on('pan', function(){ console.log('PAN:', cy.pan()); });\n";
    if(manualPositions){
        html << "setTimeout(() => { cy.fit(undefined, 60); cy.zoom(1.15); cy.center(); }, 200);\n";
    } else {
        html << "setTimeout(() => { cy.fit(undefined, 120); cy.center(); cy.zoom(1.0); }, 250);\n";
    }

    // Highlight path if exists
    html << "function highlightPath(path){ if(!path || path.length==0) return; for(let i=0;i<path.length-1;i++){ let e = cy.edges().filter(function(ed){ return ed.data('source')===path[i] && ed.data('target')===path[i+1]; }); e.addClass('highlighted'); } // mark start/end nodes\n";
    html << "cy.$id(path[0]).addClass('start'); cy.$id(path[path.length-1]).addClass('end'); }\n";

    // Robot animation: change node labels to show robot emoji on current node and flag at destination
    html << "function animateRobot(path){ if(!path || path.length==0) return; let idx=0; function step(){ if(idx>0){ // restore previous node label\n";
    html << " let prev = path[idx-1]; cy.$id(prev).data('label', prev); } let cur = path[idx]; // set robot on cur\n";
    html << " cy.$id(cur).data('label', '🤖 '+cur); if(idx==path.length-1){ // destination flag\n";
    html << " cy.$id(cur).data('label', cur + ' 🚩'); return; } idx++; setTimeout(step, 700); } // start: label start with robot\n";
    html << " cy.$id(path[0]).data('label', '🤖 ' + path[0]); setTimeout(step, 700); }\n";

    html << "// apply highlighting and animation when shortestPath present\n";
    html << "if(shortestPath.length>0){ highlightPath(shortestPath); animateRobot(shortestPath); }\n";

    html << "</script>\n";

    html << "</body>\n</html>\n";

    html.close();
}

// Helper: read graph for custom mode
bool nodeExists(const string& n){
    return find(nodes.begin(), nodes.end(), n) != nodes.end();
}

void normalizeNodeInput(string& value){
    transform(value.begin(), value.end(), value.begin(), [](unsigned char ch){
        return static_cast<char>(toupper(ch));
    });
}

string toUpperCopy(string value){
    return toUpperCase(value);
}

bool tryResolveNodeCaseInsensitive(const string& input, string& resolved){
    string inputUpper = toUpperCopy(input);
    for(const auto& n : nodes){
        if(toUpperCopy(n) == inputUpper){
            resolved = n;
            return true;
        }
    }
    return false;
}

int main(){
 
    ios::sync_with_stdio(false);

    system("cls");

    while(true){
        cout << "====================================\n";
        cout << "IMPLEMENTASI ALGORITMA DIJKSTRA\n";
        cout << "VISUAL GAME PATH FINDING\n";
        cout << "====================================\n\n";
        cout << "1. Mode Predefined Graph\n2. Mode Custom Graph\n0. Exit\n";
        cout << "Pilih mode: ";
        int mode;
        while(true){
            if(readMenuChoice(mode)) break;
            cout << "[ERROR]\nInput tidak valid.\nMasukkan angka 0, 1, atau 2.\n";
            cout << "Pilih mode: ";
        }
        if(mode==0) return 0;

        if(mode==1){
            setupPredefinedGraph();

            // Manual positions for mode 1 (fixed map-like layout)
            map<string, pair<int,int>> pos = {
                {"V1",{180,450}},
                {"V2",{360,240}},
                {"V3",{300,560}},
                {"V4",{360,660}},
                {"V5",{600,240}},
                {"V6",{600,450}},
                {"V7",{600,660}},
                {"V8",{880,900}},
                {"V9",{840,120}},
                {"V10",{840,340}},
                {"V11",{860,540}},
                {"V12",{900,720}},
                {"V13",{1100,280}},
                {"V14",{1320,560}}
            };

            // show initial graph first
            vector<string> emptyPath;
            generateHTML(emptyPath, 0, "", "", true, pos);
            system("start index.html");

            string source, destination;
            cout << "\n==================================\n";
            cout << "INPUT NODE\n";
            cout << "==================================\n\n";
            cout << "Format node:\nV1 sampai V14\n\n";
            cout << "Contoh:\nNode awal : V1\nNode tujuan : V14\n\n";
            while(true){
                if(!readSingleTokenLine("Masukkan node awal: ", source) || !readSingleTokenLine("Masukkan node tujuan: ", destination)){
                    cout << "\n❌ Node tidak valid.\n";
                    cout << "Silakan input ulang.\n\n";
                    continue;
                }

                string resolvedSource, resolvedDestination;
                bool sourceValid = tryResolveNodeCaseInsensitive(source, resolvedSource);
                bool destinationValid = tryResolveNodeCaseInsensitive(destination, resolvedDestination);

                if(sourceValid && destinationValid){
                    source = resolvedSource;
                    destination = resolvedDestination;
                    break;
                }

                cout << "\n❌ Node tidak valid.\n";
                cout << "Silakan input ulang.\n\n";
                cout << "Node tersedia:\n";
                for(size_t i=0;i<nodes.size();++i){
                    cout << nodes[i];
                    if(i+1!=nodes.size()) cout << ", ";
                    if((i+1)%6==0) cout << "\n";
                }
                cout << "\n\n";
            }

            auto result = dijkstra(source);
            auto dist = result.first;
            auto prev = result.second;

            vector<string> path = reconstructPath(prev, destination);
            int total = dist[destination] >= INF ? -1 : dist[destination];

            if(path.empty() || total==-1){
                cout << "Tidak ada jalur dari " << source << " ke " << destination << "\n";
            } else {
                cout << "\nShortest Path:\n";
                for(size_t i=0;i<path.size();++i){ cout << path[i]; if(i+1!=path.size()) cout << " -> "; }
                cout << "\nTotal Bobot: " << total << "\n";
            }

            generateHTML(path, total, source, destination, true, pos);
            system("start index.html");

        } else if(mode==2){
            graph.clear(); nodes.clear(); processSteps.clear();
            int n;
            while(true){
                if(!readPositiveIntLine("Masukkan jumlah node: ", n)){
                    cout << "[ERROR]\nJumlah node harus bilangan bulat positif.\n";
                    continue;
                }
                if(n < 2){
                    cout << "[ERROR]\nJumlah node minimal 2.\nSilakan input ulang.\n";
                    continue;
                }
                break;
            }
            cout << "\n==================================\n";
            cout << "INPUT NODE GRAPH\n";
            cout << "==================================\n\n";
            nodes.resize(n);
            for(int i=0;i<n;++i){
                while(true){
                    string rawNode;
                    cout << "Masukkan node ke-" << (i+1) << ": ";
                    if(!readSingleTokenLine("", rawNode)){
                        cout << "\n[ERROR]\nNama node tidak valid.\nSilakan input nama node lain.\n\n";
                        continue;
                    }

                    if(!containsAlphabet(rawNode)){
                        cout << "\n[ERROR]\nNama node tidak valid.\nNode harus mengandung minimal satu huruf.\nSilakan input nama node lain.\n\n";
                        continue;
                    }

                    string normalizedNode = toLowerCase(rawNode);
                    if(find(nodes.begin(), nodes.begin() + i, normalizedNode) != nodes.begin() + i){
                        cout << "\n[ERROR]\nNode \"" << rawNode << "\" sudah ada.\nSilakan input nama node lain.\n\n";
                        continue;
                    }

                    nodes[i] = normalizedNode;
                    break;
                }
            }

            int m;
            cout << "\n==================================\n";
            cout << "INPUT EDGE GRAPH\n";
            cout << "==================================\n\n";
            int maxEdge = n * (n - 1);
            while(true){
                if(readStrictIntLine("Masukkan jumlah edge: ", m) && m >= 0 && m <= maxEdge) break;
                cout << "[ERROR]\nJumlah edge melebihi batas maksimum.\n\nUntuk " << n << " node,\nmaksimal edge adalah " << maxEdge << ".\n\nSilakan input ulang.\n";
            }
            for(int i=0;i<m;++i){
                while(true){
                    cout << "Edge ke-" << (i+1) << "\n";
                    cout << "Source destination weight: ";

                    string rawSource, rawDestination;
                    int w;
                    if(!readEdgeLine("", rawSource, rawDestination, w)){
                        cout << "\n[ERROR]\nFormat input salah.\nGunakan format:\nsource destination weight\n\nSilakan input ulang edge.\n\n";
                        continue;
                    }

                    string a = toLowerCase(rawSource);
                    string b = toLowerCase(rawDestination);

                    if(!nodeExists(a) || !nodeExists(b)){
                        cout << "\n[ERROR]\nNode tidak ditemukan.\nSilakan input ulang edge.\n\n";
                        continue;
                    }

                    if(a == b){
                        cout << "\n[ERROR]\nNode tidak boleh terhubung ke dirinya sendiri.\nInput ulang.\n\n";
                        continue;
                    }

                    if(w <= 0){
                        cout << "\n[ERROR]\nWeight harus bilangan bulat positif.\nSilakan input ulang edge.\n\n";
                        continue;
                    }

                    if(directedEdgeExists(a, b)){
                        cout << "\n[ERROR]\nEdge " << toUpperCase(a) << " -> " << toUpperCase(b) << " sudah ada.\nSilakan input edge lain.\n\n";
                        continue;
                    }

                    addDirectedEdge(a, b, w);
                    break;
                }
            }

            // show initial graph (auto layout)
            vector<string> emptyPath;
            map<string, pair<int,int>> noPos;
            generateHTML(emptyPath, 0, "", "", false, noPos);
            system("start index.html");

            string source, destination;
            cout << "\n==================================\n";
            cout << "INPUT NODE\n";
            cout << "==================================\n\n";
            cout << "Format node:\n";
            for(size_t i=0;i<nodes.size();++i){
                cout << toUpperCase(nodes[i]);
                if(i+1!=nodes.size()) cout << ", ";
                if((i+1)%6==0) cout << "\n";
            }
            cout << "\n\nContoh:\nNode awal : " << toUpperCase(nodes.front()) << "\nNode tujuan : " << toUpperCase(nodes.back()) << "\n\n";
            while(true){
                if(!readSingleTokenLine("Masukkan node awal: ", source) || !readSingleTokenLine("Masukkan node tujuan: ", destination)){
                    cout << "\n❌ Node tidak valid.\n";
                    cout << "Silakan input ulang.\n\n";
                    continue;
                }

                source = toLowerCase(source);
                destination = toLowerCase(destination);

                string resolvedSource, resolvedDestination;
                bool sourceValid = tryResolveNodeCaseInsensitive(source, resolvedSource);
                bool destinationValid = tryResolveNodeCaseInsensitive(destination, resolvedDestination);

                if(sourceValid && destinationValid){
                    source = resolvedSource;
                    destination = resolvedDestination;
                    break;
                }

                cout << "\n❌ Node tidak valid.\n";
                cout << "Silakan input ulang.\n\n";
                cout << "Node tersedia:\n";
                for(size_t i=0;i<nodes.size();++i){
                    cout << nodes[i];
                    if(i+1!=nodes.size()) cout << ", ";
                    if((i+1)%6==0) cout << "\n";
                }
                cout << "\n\n";
            }

            auto result = dijkstra(source);
            auto dist = result.first;
            auto prev = result.second;

            vector<string> path = reconstructPath(prev, destination);
            int total = dist[destination] >= INF ? -1 : dist[destination];

            if(path.empty() || total==-1){
                cout << "Tidak ada jalur dari " << toUpperCase(source) << " ke " << toUpperCase(destination) << "\n";
            } else {
                cout << "\nShortest Path:\n";
                for(size_t i=0;i<path.size();++i){ cout << toUpperCase(path[i]); if(i+1!=path.size()) cout << " -> "; }
                cout << "\nTotal Bobot: " << total << "\n";
            }

            map<string, pair<int,int>> noPos2;
            generateHTML(path, total, source, destination, false, noPos2);
            system("start index.html");

        } else {
            cout << "Mode tidak dikenal. Coba lagi.\n";
        }
    }

    return 0;
}
