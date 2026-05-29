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
#include <iomanip>
#include <string>

using namespace std;

const int INF = 1e9;

struct Edge {
    string to;
    int weight;
};

struct Step {
    int iteration;
    string current;
    vector<string> visited;                  // in-time visit order
    map<string, pair<int, string>> state;    // node -> (dist, prev)
    map<string, int> updatedAt;              // node -> iteration when value was last updated
    vector<pair<int,string>> queueSnapshot;  // isi queue sebelum node ini diambil
    vector<string> neighborsChecked;         // tetangga yang dicek iterasi ini
};

map<string, vector<Edge>> graph;
vector<string> nodes;
vector<Step> processSteps;

// ============================================================
// A. HTML & DESIGN
// ============================================================

// Helper: JS array builder (dipakai di writeHTMLScripts)
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

// Helper: write <head> section (meta, script, title)
void writeHTMLHead(ofstream& html){
    html << "<!DOCTYPE html>\n<html>\n<head>\n<meta charset=\"UTF-8\">\n";
    html << "<script src=\"https://unpkg.com/cytoscape/dist/cytoscape.min.js\"></script>\n";
    html << "<script src=\"https://unpkg.com/layout-base/layout-base.js\"></script>\n";
    html << "<script src=\"https://unpkg.com/cose-base/cose-base.js\"></script>\n";
    html << "<script src=\"https://unpkg.com/cytoscape-fcose/cytoscape-fcose.js\"></script>\n";
    html << "<title>Dijkstra Game Visualization</title>\n";
}

// Helper: write <style> section
void writeHTMLStyles(ofstream& html){
    html << "<style>\n";
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
    html << ".anim-current { background-color: #facc15 !important; border-color: #d97706 !important; }\n";
    html << ".anim-visited { background-color: #86efac !important; border-color: #16a34a !important; }\n";
    html << ".anim-relaxed { line-color: #f97316 !important; target-arrow-color: #f97316 !important; width: 5px !important; }\n";
    html << "</style>\n</head>\n<body>\n";
}

// Helper: write top section (title, legend, guide, result)
void writeHTMLTopSection(ofstream& html,
                          const vector<string>& shortestPath,
                          int totalWeight,
                          const string& source,
                          const string& destination){
    // forward-declare toUpperCase usage — defined in section B
    auto uc = [](string s){ transform(s.begin(),s.end(),s.begin(),::toupper); return s; };

    html << "<div class='page'>\n";
    html << "<div class='top-section'>\n";
    html << "<h1>🤖 Mekorama Inspired — Dijkstra Visualization</h1>\n";

    // Legend
    html << "<div class='legend'>\n";
    html << "<span class='legend-item'><span class='legend-dot dot-start'></span>Start Node</span>\n";
    html << "<span class='legend-item'><span class='legend-dot dot-end'></span>Goal Node</span>\n";
    html << "<span class='legend-item'><span class='legend-dot dot-node'></span>Normal Node</span>\n";
    html << "</div>\n";

    // Guide
    html << "<div class='guide'>\n";
    html << "<h3>Cara Input Node</h3>\n";
    html << "<p>Masukkan node dengan format yang benar. Jalur akan dicari otomatis menggunakan algoritma Dijkstra.</p>\n";
    if(!source.empty() && !destination.empty()){
        html << "<p>Node awal: <strong>" << uc(source) << "</strong> | Node tujuan: <strong>" << uc(destination) << "</strong></p>\n";
    }
    html << "<p class='nodes'>Node tersedia: ";
    for(size_t i = 0; i < nodes.size(); ++i){
        html << uc(nodes[i]);
        if(i + 1 != nodes.size()) html << ", ";
    }
    html << "</p>\n</div>\n";

    // Result area
    html << "<div class='result' id='resultArea'>\n";
    if(!shortestPath.empty()){
        html << "<h2>Shortest Path</h2>\n<p>";
        for(size_t i=0;i<shortestPath.size();++i){
            html << uc(shortestPath[i]);
            if(i+1!=shortestPath.size()) html << " → ";
        }
        html << "</p>\n<p>Total Bobot: " << totalWeight << "</p>\n";
    } else {
        html << "<h2>Graph Preview</h2><p>Graph ditampilkan. Masukkan node awal & tujuan di console.</p>\n";
    }
    html << "</div>\n";
    html << "</div>\n";
}

// Helper: write graph container section
void writeHTMLGraphSection(ofstream& html){
    html << "<div class='graph-section'>\n<div class='graph-container'>\n<div id=\"cy\"></div>\n</div>\n</div>\n";
}

// Helper: write animation panel HTML (tombol + tabel dinamis di bawahnya)
void writeHTMLAnimationPanel(ofstream& html){
    html << "<div id='anim-panel' style='margin:16px auto;max-width:1100px;padding:16px 20px;\n";
    html << "background:#f8fafc;border:1px solid #e5e7eb;border-radius:16px;'>\n";
    html << "  <h3 style='margin:0 0 10px;font-size:16px;'>🎮 Step-by-Step Dijkstra Playback</h3>\n";
    html << "  <div id='anim-info' style='min-height:40px;padding:10px 14px;background:#fff;\n";
    html << "  border:1px solid #e5e7eb;border-radius:10px;font-size:14px;margin-bottom:12px;\n";
    html << "  color:#374151;'>Tekan Play atau Next untuk mulai animasi.</div>\n";
    html << "  <div style='display:flex;gap:10px;align-items:center;flex-wrap:wrap;'>\n";
    html << "    <button onclick='animPrev()' style='padding:8px 18px;border-radius:8px;\n";
    html << "    border:1px solid #d1d5db;background:#fff;cursor:pointer;font-size:14px;'>⏮ Prev</button>\n";
    html << "    <button id='btn-play' onclick='animPlayPause()' style='padding:8px 18px;\n";
    html << "    border-radius:8px;border:none;background:#3b82f6;color:#fff;cursor:pointer;\n";
    html << "    font-size:14px;'>▶ Play</button>\n";
    html << "    <button onclick='animNext()' style='padding:8px 18px;border-radius:8px;\n";
    html << "    border:1px solid #d1d5db;background:#fff;cursor:pointer;font-size:14px;'>Next ⏭</button>\n";
    html << "    <button onclick='animReset()' style='padding:8px 18px;border-radius:8px;\n";
    html << "    border:1px solid #d1d5db;background:#fff;cursor:pointer;font-size:14px;'>↺ Reset</button>\n";
    html << "    <label style='font-size:13px;color:#6b7280;margin-left:8px;'>Speed:</label>\n";
    html << "    <input type='range' id='anim-speed' min='1' max='3' value='2' style='cursor:pointer;'>\n";
    html << "    <span style='font-size:13px;color:#6b7280;' id='speed-label'>Normal</span>\n";
    html << "    <span style='font-size:13px;color:#6b7280;margin-left:auto;'>Step: <strong id='anim-step-num'>0</strong> / <strong id='anim-total-steps'>0</strong></span>\n";
    html << "  </div>\n";

    // Tabel dinamis — muncul tepat di bawah tombol, update tiap Next/Prev
    html << "  <div id='step-detail-wrap' style='margin-top:16px;display:none;'>\n";
    html << "    <h4 style='margin:0 0 8px;font-size:14px;color:#374151;'>Detail iterasi ini</h4>\n";
    html << "    <div style='overflow-x:auto;'>\n";
    html << "    <table id='step-detail-table' style='width:100%;border-collapse:collapse;font-size:12px;background:#fff;border:1px solid #e5e7eb;border-radius:8px;overflow:hidden;'>\n";
    html << "      <thead>\n";
    html << "        <tr style='background:#f1f5f9;'>\n";
    html << "          <th style='padding:7px 10px;border:1px solid #e5e7eb;text-align:left;white-space:nowrap;min-width:90px;'>Node diproses</th>\n";
    html << "          <th style='padding:7px 10px;border:1px solid #e5e7eb;text-align:left;min-width:200px;'>Pertimbangan queue (sebelum dipilih)</th>\n";
    html << "          <th style='padding:7px 10px;border:1px solid #e5e7eb;text-align:left;min-width:140px;'>Tetangga dicek</th>\n";
    html << "          <th style='padding:7px 10px;border:1px solid #e5e7eb;text-align:left;min-width:180px;'>Perubahan jarak</th>\n";
    html << "          <th style='padding:7px 10px;border:1px solid #e5e7eb;text-align:left;min-width:200px;'>Belum dikunjungi</th>\n";
    html << "        </tr>\n";
    html << "      </thead>\n";
    html << "      <tbody id='step-detail-body'>\n";
    html << "        <tr><td colspan='5' style='text-align:center;color:#9ca3af;padding:12px;'>Tekan Next untuk melihat detail.</td></tr>\n";
    html << "      </tbody>\n";
    html << "    </table>\n";
    html << "    </div>\n";

    // Legend warna tabel dinamis
    html << "    <div style='display:flex;gap:14px;flex-wrap:wrap;margin-top:8px;font-size:11px;color:#6b7280;'>\n";
    html << "      <span><span style='display:inline-block;width:10px;height:10px;background:#facc15;border:1px solid #d97706;border-radius:2px;margin-right:4px;'></span>Node dipilih dari queue</span>\n";
    html << "      <span><span style='display:inline-block;width:10px;height:10px;background:#bfdbfe;border:1px solid #60a5fa;border-radius:2px;margin-right:4px;'></span>Jarak sama (tiebreaker alphabet)</span>\n";
    html << "      <span><span style='display:inline-block;width:10px;height:10px;background:#dcfce7;border:1px solid #86efac;border-radius:2px;margin-right:4px;'></span>Jarak diperbarui</span>\n";
    html << "      <span><span style='display:inline-block;width:10px;height:10px;background:#f3f4f6;border:1px solid #d1d5db;border-radius:2px;margin-right:4px;'></span>Sudah dikunjungi / tidak berubah</span>\n";
    html << "    </div>\n";
    html << "  </div>\n";

    html << "</div>\n";
}

// Helper: write the Dijkstra process table (statis — tidak disentuh)
void writeHTMLTable(ofstream& html, const vector<string>& shortestPath){
    auto uc = [](string s){ transform(s.begin(),s.end(),s.begin(),::toupper); return s; };

    html << "<div class='table-section'>\n<h2>Tabel Proses Dijkstra</h2>\n<div class='table-wrap'>\n<table>\n<thead>\n";
    html << "<tr><th>Iteration</th><th>Unvisited(Q)</th><th>Visited(S)</th><th>Current</th><th colspan='" << nodes.size() << "'>Node : Min = (dist[node], prev[node])</th></tr>\n";
    html << "<tr><th></th><th></th><th></th><th></th>";
    for(auto &n : nodes) html << "<th class='node-col'>" << uc(n) << "</th>";
    html << "</tr>\n</thead>\n<tbody>\n";

    auto formatNodeSet = [&](const vector<string>& list){
        stringstream ss;
        ss << "{";
        for(size_t i=0;i<list.size();++i){
            ss << uc(list[i]);
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
        html << "<td class='current-cell'>" << uc(step.current) << "</td>";

        for(auto &n : nodes){
            auto s = step.state.at(n);
            string distStr = s.first >= INF ? string("∞") : to_string(s.first);
            string prevStr = s.second.empty() ? string("-") : uc(s.second);
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
}

// Helper: write JavaScript (cytoscape, animasi, tabel dinamis)
void writeHTMLScripts(ofstream& html,
                       const vector<string>& shortestPath,
                       bool manualPositions,
                       const map<string, pair<int,int>>& positions){
    auto uc = [](string s){ transform(s.begin(),s.end(),s.begin(),::toupper); return s; };

    html << "<script>\n";

    // shortestPath array
    html << "let shortestPath = " << vectorToJSArray(shortestPath) << ";\n";

    // Inject processStepsData
    html << "let processStepsData = [\n";
    for(auto& step : processSteps){
        html << "{ iteration:" << step.iteration << ", current:'" << step.current << "',";
        html << " visited:[";
        for(size_t i=0;i<step.visited.size();i++){
            html << "'" << step.visited[i] << "'";
            if(i+1!=step.visited.size()) html << ",";
        }
        html << "], updatedNodes:[";
        bool first = true;
        for(auto& n : nodes){
            if(step.updatedAt.at(n) == step.iteration){
                if(!first) html << ",";
                html << "{node:'" << n << "',dist:" << step.state.at(n).first
                     << ",prev:'" << step.state.at(n).second << "'}";
                first = false;
            }
        }
        html << "],";

        // inject queueSnapshot
        html << " queueSnapshot:[";
        for(size_t qi=0; qi<step.queueSnapshot.size(); qi++){
            html << "{dist:" << step.queueSnapshot[qi].first
                 << ",node:'" << step.queueSnapshot[qi].second << "'}";
            if(qi+1!=step.queueSnapshot.size()) html << ",";
        }
        html << "],";

        // inject neighbors checked
        html << " neighborsChecked:[";
        for(size_t ni=0; ni<step.neighborsChecked.size(); ni++){
            html << "'" << step.neighborsChecked[ni] << "'";
            if(ni+1!=step.neighborsChecked.size()) html << ",";
        }
        html << "],";

        // inject unvisited at this step
        html << " unvisited:[";
        {
            unordered_set<string> vis(step.visited.begin(), step.visited.end());
            bool fv = true;
            for(auto& n : nodes){
                if(!vis.count(n)){
                    if(!fv) html << ",";
                    int d = step.state.at(n).first;
                    html << "{node:'" << n << "',dist:" << d << "}";
                    fv = false;
                }
            }
        }
        html << "]";

        html << " },\n";
    }
    html << "];\n";
    html << "let isManualPositions = " << (manualPositions ? "true" : "false") << ";\n";

    // nodes & edges data untuk cytoscape
    html << "let elements = [\n";
    for(auto &n : nodes){
        html << "{ data: { id: '" << n << "', label: '" << uc(n) << "' }";
        if(manualPositions){
            auto it = positions.find(n);
            if(it!=positions.end()){
                html << ", position: { x: " << it->second.first << ", y: " << it->second.second << " }";
            }
        }
        html << " },\n";
    }
    for(auto &p : graph){
        for(auto &e : p.second){
            html << "{ data: { source: '" << p.first << "', target: '" << e.to << "', label: '" << e.weight << "' } },\n";
        }
    }
    html << "];\n";

    // cytoscape init
    html << "var cy = cytoscape({ container: document.getElementById('cy'), pixelRatio: 1, wheelSensitivity: 0.2, ";
    html << "autoungrabify: true, ";
    html << "userPanningEnabled: true, userZoomingEnabled: true, boxSelectionEnabled: false, elements: elements, style: [ { selector:'node', style:{ 'label':'data(label)','background-color':'#4FC3F7','color':'#111827','font-size':16,'font-weight':'bold','text-valign':'center','text-halign':'center','width':65,'height':65,'border-width':3,'border-color':'#1f2937' } }, ";
    if(manualPositions){
        html << "{ selector:'edge', style:{ 'label':'data(label)','curve-style':'unbundled-bezier','control-point-distances':[60],'control-point-weights':[0.5],'width':3,'line-color':'#6b7280','target-arrow-color':'#6b7280','target-arrow-shape':'triangle','source-endpoint':'outside-to-node','target-endpoint':'outside-to-node','font-size':13,'font-weight':'bold','color':'#111827','text-background-color':'#ffffff','text-background-opacity':1,'text-background-padding':4,'text-border-color':'#d1d5db','text-border-width':1,'text-rotation':'autorotate','loop-direction':'0deg','loop-sweep':'45deg' } }, ";
    } else {
        html << "{ selector:'edge', style:{ 'label':'data(label)','curve-style':'bezier','control-point-step-size':80,'width':3,'line-color':'#6b7280','target-arrow-color':'#6b7280','arrow-scale':1.2,'target-arrow-shape':'triangle','source-endpoint':'outside-to-node','target-endpoint':'outside-to-node','font-size':14,'font-weight':'bold','color':'#111827','text-background-color':'#ffffff','text-background-opacity':1,'text-background-padding':3,'text-border-color':'#d1d5db','text-border-width':1,'text-rotation':'autorotate','text-margin-y':-8 } }, ";
    }
    html << "{ selector:'.highlighted', style:{ 'line-color':'#f59e0b','target-arrow-color':'#f59e0b','width':7 } }, ";
    html << "{ selector:'.anim-current', style:{ 'background-color':'#facc15','border-color':'#d97706','border-width':4 } }, ";
    html << "{ selector:'.anim-visited', style:{ 'background-color':'#86efac','border-color':'#16a34a' } }, ";
    html << "{ selector:'.anim-relaxed', style:{ 'line-color':'#f97316','target-arrow-color':'#f97316','width':6 } }, ";
    html << "{ selector:'.start', style:{ 'background-color':'#22c55e' } }, { selector:'.end', style:{ 'background-color':'#ef4444' } } ], minZoom:0.8, maxZoom:2.5, ";

    if(manualPositions){
        html << "layout:{ name:'preset', fit:false, padding:0 }";
    } else {
        html << "layout:{ name:'fcose', animate:false, randomize:true, fit:true, padding:120, nodeDimensionsIncludeLabels:true, idealEdgeLength:250, nodeRepulsion:80000, edgeElasticity:0.45, gravity:0.25, gravityRange:3.8, numIter:5000, tilingPaddingVertical:80, tilingPaddingHorizontal:80, uniformNodeDimensions:false, packComponents:true, samplingType:true, sampleSize:50, nodeSeparation:200 }";
    }

    html << " });\n";
    html << "cy.userPanningEnabled(true);\n";
    html << "cy.panningEnabled(true);\n";
    html << "cy.on('layoutstop', function(){ cy.nodes().lock(); });\n";
    // Separate parallel/overlapping edges in Mode 1
    if(manualPositions){
        html << "cy.ready(function(){\n";
        html << "  var edgePairs = {};\n";
        html << "  cy.edges().forEach(function(e){\n";
        html << "    var s = e.data('source'), t = e.data('target');\n";
        html << "    var key = [s,t].sort().join('_');\n";
        html << "    if(!edgePairs[key]) edgePairs[key] = [];\n";
        html << "    edgePairs[key].push(e);\n";
        html << "  });\n";
        html << "  Object.keys(edgePairs).forEach(function(key){\n";
        html << "    var edges = edgePairs[key];\n";
        html << "    var n = edges.length;\n";
        html << "    var baseOffset = 70;\n";
        html << "    edges.forEach(function(e, i){\n";
        html << "      var offset = (i - (n-1)/2.0) * baseOffset;\n";
        html << "      e.style({'curve-style':'unbundled-bezier','control-point-distances':[offset],'control-point-weights':[0.5]});\n";
        html << "    });\n";
        html << "  });\n";
        html << "});\n";
    }
    if(manualPositions){
        html << "setTimeout(() => { cy.fit(undefined, 60); cy.zoom(1.15); cy.center(); }, 200);\n";
    } else {
        html << "setTimeout(() => { cy.fit(undefined, 120); cy.center(); cy.zoom(1.0); }, 250);\n";
    }

    // Highlight path
    html << "function highlightPath(path){ if(!path || path.length==0) return; for(let i=0;i<path.length-1;i++){ let e = cy.edges().filter(function(ed){ return ed.data('source')===path[i] && ed.data('target')===path[i+1]; }); e.addClass('highlighted'); }\n";
    html << "cy.$id(path[0]).addClass('start'); cy.$id(path[path.length-1]).addClass('end'); }\n";

    // Robot animation
    html << "function animateRobot(path){\n";
    html << "  if(!path || path.length==0) return;\n";
    html << "  cy.nodes().forEach(n => { n.data('label', n.id()); });\n";
    html << "  let idx = 0;\n";
    html << "  function step(){\n";
    html << "    if(idx > 0){\n";
    html << "      let prev = path[idx-1];\n";
    html << "      cy.$id(prev).data('label', prev);\n";
    html << "    }\n";
    html << "    let cur = path[idx];\n";
    html << "    if(idx == path.length - 1){\n";
    html << "      cy.$id(cur).data('label', cur + ' 🚩');\n";
    html << "      return;\n";
    html << "    }\n";
    html << "    cy.$id(cur).data('label', '🤖 ' + cur);\n";
    html << "    idx++;\n";
    html << "    setTimeout(step, 700);\n";
    html << "  }\n";
    html << "  setTimeout(step, 300);\n";
    html << "}\n";

    html << "if(shortestPath.length>0){ highlightPath(shortestPath); animateRobot(shortestPath); }\n";

    // Playback controls
    html << "let animStepIndex = 0;\n";
    html << "let animInterval = null;\n";
    html << "function getSpeedDelay() {\n";
    html << "  let val = document.getElementById('anim-speed').value;\n";
    html << "  let label = document.getElementById('speed-label');\n";
    html << "  if (val == 1) {\n";
    html << "    label.textContent = 'Slow';\n";
    html << "    return 1500;\n";
    html << "  } else if (val == 2) {\n";
    html << "    label.textContent = 'Normal';\n";
    html << "    return 800;\n";
    html << "  } else {\n";
    html << "    label.textContent = 'Fast';\n";
    html << "    return 300;\n";
    html << "  }\n";
    html << "}\n";
    html << "document.getElementById('anim-speed').addEventListener('input', () => {\n";
    html << "  getSpeedDelay();\n";
    html << "  if (animInterval) {\n";
    html << "    animStop();\n";
    html << "    animPlay();\n";
    html << "  }\n";
    html << "});\n";
    html << "document.getElementById('anim-total-steps').textContent = processStepsData.length;\n";

    // renderStepTable — tabel dinamis per iterasi
    html << "function renderStepTable(idx) {\n";
    html << "  var wrap = document.getElementById('step-detail-wrap');\n";
    html << "  var tbody = document.getElementById('step-detail-body');\n";
    html << "  if (idx < 0 || idx >= processStepsData.length) {\n";
    html << "    wrap.style.display = 'none';\n";
    html << "    tbody.innerHTML = '<tr><td colspan=\"5\" style=\"text-align:center;color:#9ca3af;padding:12px;\">Tekan Next untuk melihat detail.</td></tr>';\n";
    html << "    return;\n";
    html << "  }\n";
    html << "  wrap.style.display = 'block';\n";
    html << "  var step = processStepsData[idx];\n";
    html << "  var INF = 1e9;\n";
    html << "  var ds = function(v){ return v >= INF ? '∞' : v; };\n";

    // Kolom 1: node diproses
    html << "  var col1 = '<span style=\"display:inline-block;padding:2px 10px;border-radius:12px;background:#facc15;color:#111;font-weight:700;border:1px solid #d97706;\">' + step.current.toUpperCase() + '</span>';\n";

    // Kolom 2: pertimbangan queue
    html << "  var minDist = Math.min.apply(null, step.queueSnapshot.map(function(q){return q.dist;}));\n";
    html << "  var col2 = step.queueSnapshot.map(function(q){\n";
    html << "    var isPicked = q.node === step.current;\n";
    html << "    var isTie = !isPicked && q.dist === minDist;\n";
    html << "    var label = q.node.toUpperCase() + '=' + ds(q.dist);\n";
    html << "    if (isPicked) return '<span style=\"padding:1px 7px;border-radius:10px;background:#facc15;color:#111;font-weight:700;border:1px solid #d97706;white-space:nowrap;\">' + label + ' ▲</span>';\n";
    html << "    if (isTie)   return '<span style=\"padding:1px 7px;border-radius:10px;background:#bfdbfe;color:#1e3a8a;border:1px solid #60a5fa;white-space:nowrap;\">' + label + '</span>';\n";
    html << "    return '<span style=\"padding:1px 5px;color:#374151;white-space:nowrap;\">' + label + '</span>';\n";
    html << "  }).join(' ');\n";
    html << "  if (!col2) col2 = '<span style=\"color:#9ca3af;\">—</span>';\n";

    // Kolom 3: tetangga dicek
    html << "  var col3 = step.neighborsChecked.map(function(n){\n";
    html << "    return '<span style=\"padding:1px 7px;border-radius:10px;background:#e0e7ff;color:#3730a3;border:1px solid #a5b4fc;white-space:nowrap;\">' + n.toUpperCase() + '</span>';\n";
    html << "  }).join(' ');\n";
    html << "  if (!col3) col3 = '<span style=\"color:#9ca3af;\">tidak ada tetangga</span>';\n";

    // Kolom 4: perubahan jarak
    html << "  var col4 = '';\n";
    html << "  if (step.updatedNodes && step.updatedNodes.length > 0) {\n";
    html << "    col4 = step.updatedNodes.map(function(u){\n";
    html << "      return '<span style=\"padding:1px 7px;border-radius:10px;background:#dcfce7;color:#14532d;border:1px solid #86efac;white-space:nowrap;\">' + u.node.toUpperCase() + '=' + ds(u.dist) + ' via ' + u.prev.toUpperCase() + '</span>';\n";
    html << "    }).join(' ');\n";
    html << "  } else {\n";
    html << "    col4 = '<span style=\"color:#9ca3af;\">tidak ada perubahan</span>';\n";
    html << "  }\n";

    // Kolom 5: belum dikunjungi
    html << "  var visitedSet = {};\n";
    html << "  step.visited.forEach(function(v){ visitedSet[v] = true; });\n";
    html << "  var col5 = step.unvisited.map(function(u){\n";
    html << "    var hasQueue = u.dist < INF;\n";
    html << "    if (hasQueue) {\n";
    html << "      return '<span style=\"padding:1px 7px;border-radius:10px;background:#e0f2fe;color:#0c4a6e;border:1px solid #7dd3fc;white-space:nowrap;\">' + u.node.toUpperCase() + '(' + ds(u.dist) + ')</span>';\n";
    html << "    } else {\n";
    html << "      return '<span style=\"padding:1px 7px;border-radius:10px;background:#f3f4f6;color:#9ca3af;border:1px solid #d1d5db;white-space:nowrap;\">' + u.node.toUpperCase() + '(∞)</span>';\n";
    html << "    }\n";
    html << "  }).join(' ');\n";
    html << "  if (!col5) col5 = '<span style=\"color:#16a34a;font-weight:600;\">Semua node sudah dikunjungi!</span>';\n";

    html << "  tbody.innerHTML = '<tr>'\n";
    html << "    + '<td style=\"padding:8px 10px;border:1px solid #e5e7eb;vertical-align:top;\">' + col1 + '</td>'\n";
    html << "    + '<td style=\"padding:8px 10px;border:1px solid #e5e7eb;vertical-align:top;line-height:1.8;\">' + col2 + '</td>'\n";
    html << "    + '<td style=\"padding:8px 10px;border:1px solid #e5e7eb;vertical-align:top;line-height:1.8;\">' + col3 + '</td>'\n";
    html << "    + '<td style=\"padding:8px 10px;border:1px solid #e5e7eb;vertical-align:top;line-height:1.8;\">' + col4 + '</td>'\n";
    html << "    + '<td style=\"padding:8px 10px;border:1px solid #e5e7eb;vertical-align:top;line-height:1.8;\">' + col5 + '</td>'\n";
    html << "    + '</tr>';\n";
    html << "}\n";

    // Play / Pause / Next / Prev / Reset / ShowStep
    html << "function animPlay() {\n";
    html << "  if (animStepIndex >= processStepsData.length) {\n";
    html << "    animReset();\n";
    html << "  }\n";
    html << "  document.getElementById('btn-play').textContent = '⏸ Pause';\n";
    html << "  animInterval = setInterval(() => {\n";
    html << "    if (animStepIndex < processStepsData.length) {\n";
    html << "      animShowStep(animStepIndex);\n";
    html << "      animStepIndex++;\n";
    html << "    } else {\n";
    html << "      animStop();\n";
    html << "    }\n";
    html << "  }, getSpeedDelay());\n";
    html << "}\n";
    html << "function animStop() {\n";
    html << "  if (animInterval) {\n";
    html << "    clearInterval(animInterval);\n";
    html << "    animInterval = null;\n";
    html << "  }\n";
    html << "  document.getElementById('btn-play').textContent = '▶ Play';\n";
    html << "}\n";
    html << "function animPlayPause() {\n";
    html << "  if (animInterval) {\n";
    html << "    animStop();\n";
    html << "  } else {\n";
    html << "    animPlay();\n";
    html << "  }\n";
    html << "}\n";
    html << "function animNext() {\n";
    html << "  animStop();\n";
    html << "  if (animStepIndex < processStepsData.length) {\n";
    html << "    animShowStep(animStepIndex);\n";
    html << "    animStepIndex++;\n";
    html << "  }\n";
    html << "}\n";
    html << "function animPrev() {\n";
    html << "  animStop();\n";
    html << "  if (animStepIndex > 0) {\n";
    html << "    animStepIndex--;\n";
    html << "    if (animStepIndex === 0) {\n";
    html << "      animReset();\n";
    html << "    } else {\n";
    html << "      animShowStep(animStepIndex - 1);\n";
    html << "    }\n";
    html << "  }\n";
    html << "}\n";
    html << "function animReset(){\n";
    html << "  animStop();\n";
    html << "  animStepIndex = 0;\n";
    html << "  cy.nodes().removeClass('anim-current anim-visited');\n";
    html << "  cy.edges().removeClass('anim-relaxed');\n";
    html << "  cy.nodes().forEach(n => n.data('label', n.id()));\n";
    html << "  document.getElementById('anim-info').textContent = 'Tekan Play atau Next untuk mulai animasi.';\n";
    html << "  document.getElementById('anim-step-num').textContent = '0';\n";
    html << "  renderStepTable(-1);\n";
    html << "}\n";
    html << "function animShowStep(idx) {\n";
    html << "  if (idx < 0 || idx >= processStepsData.length) return;\n";
    html << "  cy.nodes().removeClass('anim-current anim-visited');\n";
    html << "  cy.edges().removeClass('anim-relaxed');\n";
    html << "  let step = processStepsData[idx];\n";
    html << "  step.visited.forEach(nodeId => {\n";
    html << "    if (nodeId !== step.current) {\n";
    html << "      cy.$id(nodeId).addClass('anim-visited');\n";
    html << "    }\n";
    html << "  });\n";
    html << "  let currNode = cy.$id(step.current);\n";
    html << "  currNode.addClass('anim-current');\n";
    html << "  cy.nodes().forEach(n => n.data('label', n.id()));\n";
    html << "  currNode.data('label', '🤖 ' + step.current);\n";
    html << "  let infoText = 'Iteration ' + step.iteration + ' — Processing: ' + step.current.toUpperCase();\n";
    html << "  if (step.updatedNodes && step.updatedNodes.length > 0) {\n";
    html << "    infoText += ' — Updated: ';\n";
    html << "    step.updatedNodes.forEach((upNode, i) => {\n";
    html << "      if (upNode.prev) {\n";
    html << "        let edge = cy.edges().filter(ed => ed.data('source') === upNode.prev && ed.data('target') === upNode.node);\n";
    html << "        edge.addClass('anim-relaxed');\n";
    html << "      }\n";
    html << "      let distVal = upNode.dist >= 1e9 ? 'INF' : upNode.dist;\n";
    html << "      infoText += upNode.node.toUpperCase() + ' = ' + distVal + ' via ' + upNode.prev.toUpperCase();\n";
    html << "      if (i < step.updatedNodes.length - 1) infoText += ', ';\n";
    html << "    });\n";
    html << "  } else {\n";
    html << "    infoText += ' — No updates';\n";
    html << "  }\n";
    html << "  document.getElementById('anim-info').textContent = infoText;\n";
    html << "  document.getElementById('anim-step-num').textContent = idx + 1;\n";
    html << "  renderStepTable(idx);\n";
    html << "}\n";
    html << "</script>\n";
}

// generateHTML — memanggil semua helper di atas secara berurutan
void generateHTML(const vector<string>& shortestPath, int totalWeight,
                  const string& source, const string& destination,
                  bool manualPositions,
                  const map<string, pair<int,int>>& positions){
    ofstream html("index.html");

    writeHTMLHead(html);
    writeHTMLStyles(html);
    writeHTMLTopSection(html, shortestPath, totalWeight, source, destination);
    writeHTMLGraphSection(html);
    writeHTMLAnimationPanel(html);
    writeHTMLTable(html, shortestPath);
    writeHTMLScripts(html, shortestPath, manualPositions, positions);

    html << "</body>\n</html>\n";
    html.close();
}

// ============================================================
// B. UTILITIES & HELPERS
// ============================================================

// Cross-platform clear screen
void clearScreen(){
#ifdef _WIN32
    system("cls");
#else
    cout << "\033[2J\033[1;1H";
#endif
}

// Cross-platform open browser
void openBrowser(){
#ifdef _WIN32
    system("start index.html");
#elif __APPLE__
    system("open index.html");
#else
    system("xdg-open index.html");
#endif
}

// String helpers
string toLowerCase(string s){
    transform(s.begin(), s.end(), s.begin(), ::tolower);
    return s;
}

string toUpperCase(string s){
    transform(s.begin(), s.end(), s.begin(), ::toupper);
    return s;
}

string toUpperCopy(string value){
    return toUpperCase(value);
}

bool containsAlphabet(const string& value){
    for(unsigned char ch : value){
        if(isalpha(ch)) return true;
    }
    return false;
}

// Input helpers
void clearInputBuffer(){
    cin.clear();
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
}

bool readStrictIntLine(const string& prompt, int& value){
    cout << prompt;
    string line;
    if(!getline(cin, line)) return false;
    stringstream ss(line);
    long long parsed;
    string extra;
    if(!(ss >> parsed) || (ss >> extra)) return false;
    if(parsed < numeric_limits<int>::min() || parsed > numeric_limits<int>::max()) return false;
    value = static_cast<int>(parsed);
    return true;
}

bool readPositiveIntLine(const string& prompt, int& value){
    cout << prompt;
    string line;
    if(!getline(cin, line)) return false;
    stringstream ss(line);
    long long parsed;
    string extra;
    if(!(ss >> parsed) || (ss >> extra) || parsed <= 0) return false;
    if(parsed > numeric_limits<int>::max()) return false;
    value = static_cast<int>(parsed);
    return true;
}

bool readSingleTokenLine(const string& prompt, string& value){
    cout << prompt;
    string line;
    if(!getline(cin, line)) return false;
    stringstream ss(line);
    string token, extra;
    if(!(ss >> token) || (ss >> extra)) return false;
    value = token;
    return true;
}

bool readEdgeLine(const string& prompt, string& source, string& destination, int& weight){
    cout << prompt;
    string line;
    if(!getline(cin, line)) return false;
    stringstream ss(line);
    string rawSource, rawDestination;
    int parsedWeight;
    string extra;
    if(!(ss >> rawSource >> rawDestination >> parsedWeight) || (ss >> extra)) return false;
    source = rawSource;
    destination = rawDestination;
    weight = parsedWeight;
    return true;
}

bool readMenuChoice(int& mode){
    string line;
    if(!getline(cin, line)) return false;
    stringstream ss(line);
    long long parsed;
    string extra;
    if(!(ss >> parsed) || (ss >> extra)) return false;
    if(parsed < 0 || parsed > 2 || parsed > numeric_limits<int>::max()) return false;
    mode = static_cast<int>(parsed);
    return true;
}

// Node helpers
bool nodeExists(const string& n){
    return find(nodes.begin(), nodes.end(), n) != nodes.end();
}

void normalizeNodeInput(string& value){
    transform(value.begin(), value.end(), value.begin(), [](unsigned char ch){
        return static_cast<char>(toupper(ch));
    });
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

// Edge helpers
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

// Graph setup — predefined V1-V14
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

// ============================================================
// C. CONSOLE OUTPUT
// ============================================================

void printConsoleTable(const map<string,int>& dist,
                       map<string,string>& previous,
                       const string& source){
    cout << "\n";
    cout << "+======================================================================+\n";
    cout << "|           SHORTEST DISTANCES FROM " << setw(6) << left << toUpperCase(source) << "                              |\n";
    cout << "+============+===============+=========================================+\n";
    cout << "|    Node    |   Distance    |              Path                       |\n";
    cout << "+============+===============+=========================================+\n";

    for(const auto& node : nodes){
        string distStr;
        string pathStr;

        if(dist.at(node) >= INF){
            distStr = "INF";
            pathStr = "(unreachable)";
        } else {
            distStr = to_string(dist.at(node));
            vector<string> path;

            // inline reconstruct untuk printConsoleTable
            if(node == source){
                path.push_back(source);
            } else {
                unordered_set<string> vis;
                string cur = node;
                while(cur != ""){
                    if(vis.count(cur)){ path.clear(); break; }
                    vis.insert(cur);
                    path.push_back(cur);
                    cur = previous[cur];
                }
                reverse(path.begin(), path.end());
                if(!path.empty() && path.front() != source) path.clear();
            }

            if(!path.empty()){
                stringstream ss;
                for(size_t i = 0; i < path.size(); ++i){
                    ss << toUpperCase(path[i]);
                    if(i + 1 != path.size()) ss << " -> ";
                }
                pathStr = ss.str();
            } else {
                pathStr = "(no path)";
            }
        }

        cout << "| " << setw(10) << left << toUpperCase(node)
             << " | " << setw(13) << left << distStr
             << " | " << setw(39) << left << pathStr << " |\n";
    }

    cout << "+============+===============+=========================================+\n";
}

void printComplexityNote(){
    cout << "\n------------------------------------------------------\n";
    cout << "Time Complexity: O((V + E) log V) using Min-Heap Priority Queue\n";
    cout << "Space Complexity: O(V + E)\n";
    cout << "------------------------------------------------------\n";
}

// ============================================================
// D. ALGORITMA DIJKSTRA
// ============================================================

// reconstructPath — dengan cycle guard & validasi lengkap
vector<string> reconstructPath(map<string,string>& previous,
                               const map<string,int>& dist,
                               const string& source,
                               const string& destination){
    vector<string> path;

    if(dist.find(destination) == dist.end()) return path;
    if(dist.at(destination) >= INF) return path;
    if(source == destination){
        path.push_back(source);
        return path;
    }

    unordered_set<string> visited;
    string cur = destination;
    while(cur != ""){
        if(visited.count(cur)) return vector<string>();
        visited.insert(cur);
        path.push_back(cur);
        cur = previous[cur];
    }

    reverse(path.begin(), path.end());

    if(path.empty() || path.front() != source) return vector<string>();

    return path;
}

// dijkstra — algoritma inti dengan priority queue min-heap
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

    if(dist.find(source) == dist.end()) return {dist, previous};

    dist[source] = 0;

    map<string,int> lastUpdatedAt;
    for(auto &node : nodes) lastUpdatedAt[node] = -1;
    lastUpdatedAt[source] = 0;

    priority_queue< pair<int,string>, vector<pair<int,string>>, greater<pair<int,string>> > pq;
    pq.push({0, source});

    int iteration = 1;

    while(!pq.empty()){
        // Snapshot isi queue SEBELUM pop — untuk kolom "Pertimbangan queue"
        vector<pair<int,string>> qSnap;
        {
            auto pqCopy = pq;
            while(!pqCopy.empty()){
                qSnap.push_back(pqCopy.top());
                pqCopy.pop();
            }
        }

        string current = pq.top().second;
        pq.pop();

        if(visitedSet.count(current)) continue;

        visitedSet.insert(current);
        visitOrder.push_back(current);

        // Relax neighbors & catat tetangga yang dicek
        vector<string> neighborsThisStep;
        for(auto &edge : graph[current]){
            if(dist[current] == INF) continue;
            neighborsThisStep.push_back(edge.to);
            int newDist = dist[current] + edge.weight;
            if(newDist < dist[edge.to]){
                dist[edge.to] = newDist;
                previous[edge.to] = current;
                lastUpdatedAt[edge.to] = iteration;
                pq.push({newDist, edge.to});
                cout << "Update " << toUpperCase(edge.to) << " = " << dist[edge.to]
                     << " via " << toUpperCase(current) << endl;
            }
        }

        // Rekam state SETELAH semua relaxation
        Step step;
        step.iteration = iteration++;
        step.current = current;
        step.visited = visitOrder;
        step.queueSnapshot = qSnap;
        step.neighborsChecked = neighborsThisStep;

        for(auto &node : nodes){
            step.state[node]     = { dist[node], previous[node] };
            step.updatedAt[node] = lastUpdatedAt[node];
        }

        processSteps.push_back(step);
    }

    return {dist, previous};
}

// ============================================================
// E. MAIN — paling bawah
// ============================================================
int main(){

    ios::sync_with_stdio(false);
    clearScreen();

    while(true){
        cout << "====================================\n";
        cout << "IMPLEMENTASI ALGORITMA DIJKSTRA\n";
        cout << "VISUAL GAME PATH FINDING\n";
        cout << "====================================\n\n";
        cout << "1. Mode Predefined Graph (Bidirectional)\n2. Mode Custom Graph\n0. Exit\n";
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
            cout << "\n[MODE 1] Predefined Graph — Bidirectional Edges\n";

            map<string, pair<int,int>> pos = {
                {"V1", {160,420}},
                {"V2", {440,180}}, {"V3", {440,660}},
                {"V4", {440,900}},
                {"V5", {720,180}}, {"V6", {720,420}}, {"V7", {720,660}}, {"V8", {720,900}},
                {"V9", {1000,60}}, {"V10",{1000,320}},{"V11",{1000,560}},{"V12",{1000,800}},
                {"V13",{1280,180}},{"V14",{1280,560}}
            };

            vector<string> emptyPath;
            generateHTML(emptyPath, 0, "", "", true, pos);
            openBrowser();
            cout << "\n>> HTML graph generated: index.html\n";
            cout << ">> Please open index.html in your browser to view the graph.\n\n";

            string source, destination;
            cout << "==================================\n";
            cout << "INPUT NODE\n";
            cout << "==================================\n\n";
            cout << "Format node:\nV1 sampai V14\n\n";
            cout << "Contoh:\nNode awal : V1\nNode tujuan : V14\n\n";
            while(true){
                if(!readSingleTokenLine("Masukkan node awal: ", source) ||
                   !readSingleTokenLine("Masukkan node tujuan: ", destination)){
                    cout << "\n❌ Node tidak valid.\nSilakan input ulang.\n\n";
                    continue;
                }

                string resolvedSource, resolvedDestination;
                bool sourceValid      = tryResolveNodeCaseInsensitive(source, resolvedSource);
                bool destinationValid = tryResolveNodeCaseInsensitive(destination, resolvedDestination);

                if(sourceValid && destinationValid){
                    source = resolvedSource;
                    destination = resolvedDestination;
                    break;
                }

                cout << "\n❌ Node tidak valid.\nSilakan input ulang.\n\n";
                cout << "Node tersedia:\n";
                for(size_t i=0;i<nodes.size();++i){
                    cout << nodes[i];
                    if(i+1!=nodes.size()) cout << ", ";
                    if((i+1)%6==0) cout << "\n";
                }
                cout << "\n\n";
            }

            auto result = dijkstra(source);
            auto dist   = result.first;
            auto prev   = result.second;

            vector<string> path = reconstructPath(prev, dist, source, destination);
            int total = dist[destination] >= INF ? -1 : dist[destination];

            cout << "\n==================================\n";
            cout << "RESULT: " << toUpperCase(source) << " → " << toUpperCase(destination) << "\n";
            cout << "==================================\n";

            if(path.empty() || total==-1){
                cout << "Tidak ada jalur dari " << toUpperCase(source)
                     << " ke " << toUpperCase(destination) << "\n";
            } else {
                cout << "\nShortest Path:\n";
                for(size_t i=0;i<path.size();++i){
                    cout << toUpperCase(path[i]);
                    if(i+1!=path.size()) cout << " -> ";
                }
                cout << "\nTotal Bobot: " << total << "\n";
            }

            printConsoleTable(dist, prev, source);
            printComplexityNote();

            generateHTML(path, total, source, destination, true, pos);
            openBrowser();
            cout << "\n>> HTML updated: index.html\n";
            cout << ">> Please open (or refresh) index.html in your browser to view the result.\n\n";

        } else if(mode==2){
            graph.clear(); nodes.clear(); processSteps.clear();

            bool useDirected = true;
            cout << "\n==================================\n";
            cout << "GRAPH TYPE\n";
            cout << "==================================\n\n";
            cout << "1. Directed Graph   (A -> B only)\n";
            cout << "2. Undirected Graph (A <-> B both ways)\n";
            cout << "Pilih tipe graph: ";

            while(true){
                string line;
                if(!getline(cin, line)){
                    cout << "[ERROR]\nInput tidak valid. Masukkan 1 atau 2.\nPilih tipe graph: ";
                    continue;
                }
                stringstream ss(line);
                int choice;
                string extra;
                if(!(ss >> choice) || (ss >> extra) || (choice != 1 && choice != 2)){
                    cout << "[ERROR]\nInput tidak valid. Masukkan 1 atau 2.\nPilih tipe graph: ";
                    continue;
                }
                useDirected = (choice == 1);
                break;
            }

            if(useDirected){
                cout << "\n[MODE 2] Custom Graph — Directed Edges (A -> B)\n";
            } else {
                cout << "\n[MODE 2] Custom Graph — Undirected/Bidirectional Edges (A <-> B)\n";
            }

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
            cout << "INPUT EDGE GRAPH";
            cout << (useDirected ? " (Directed)" : " (Undirected)");
            cout << "\n==================================\n\n";

            int maxEdge = useDirected ? n * (n - 1) : n * (n - 1) / 2;

            while(true){
                if(readStrictIntLine("Masukkan jumlah edge: ", m) && m >= 0 && m <= maxEdge) break;
                cout << "[ERROR]\nJumlah edge melebihi batas maksimum.\n\nUntuk " << n
                     << " node,\nmaksimal edge adalah " << maxEdge << ".\n\nSilakan input ulang.\n";
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
                    if(!useDirected && directedEdgeExists(b, a)){
                        cout << "\n[ERROR]\nEdge " << toUpperCase(b) << " <-> " << toUpperCase(a) << " sudah ada.\nSilakan input edge lain.\n\n";
                        continue;
                    }
                    if(useDirected){
                        addDirectedEdge(a, b, w);
                    } else {
                        addBidirectionalEdge(a, b, w);
                    }
                    break;
                }
            }

            vector<string> emptyPath;
            map<string, pair<int,int>> noPos;
            generateHTML(emptyPath, 0, "", "", false, noPos);
            openBrowser();
            cout << "\n>> HTML graph generated: index.html\n";
            cout << ">> Please open index.html in your browser to view the graph.\n\n";

            string source, destination;
            cout << "==================================\n";
            cout << "INPUT NODE\n";
            cout << "==================================\n\n";
            cout << "Format node:\n";
            for(size_t i=0;i<nodes.size();++i){
                cout << toUpperCase(nodes[i]);
                if(i+1!=nodes.size()) cout << ", ";
                if((i+1)%6==0) cout << "\n";
            }
            cout << "\n\nContoh:\nNode awal : " << toUpperCase(nodes.front())
                 << "\nNode tujuan : " << toUpperCase(nodes.back()) << "\n\n";

            while(true){
                if(!readSingleTokenLine("Masukkan node awal: ", source) ||
                   !readSingleTokenLine("Masukkan node tujuan: ", destination)){
                    cout << "\n❌ Node tidak valid.\nSilakan input ulang.\n\n";
                    continue;
                }
                source = toLowerCase(source);
                destination = toLowerCase(destination);

                string resolvedSource, resolvedDestination;
                bool sourceValid      = tryResolveNodeCaseInsensitive(source, resolvedSource);
                bool destinationValid = tryResolveNodeCaseInsensitive(destination, resolvedDestination);

                if(sourceValid && destinationValid){
                    source = resolvedSource;
                    destination = resolvedDestination;
                    break;
                }

                cout << "\n❌ Node tidak valid.\nSilakan input ulang.\n\n";
                cout << "Node tersedia:\n";
                for(size_t i=0;i<nodes.size();++i){
                    cout << toUpperCase(nodes[i]);
                    if(i+1!=nodes.size()) cout << ", ";
                    if((i+1)%6==0) cout << "\n";
                }
                cout << "\n\n";
            }

            auto result = dijkstra(source);
            auto dist   = result.first;
            auto prev   = result.second;

            vector<string> path = reconstructPath(prev, dist, source, destination);
            int total = dist[destination] >= INF ? -1 : dist[destination];

            cout << "\n==================================\n";
            cout << "RESULT: " << toUpperCase(source) << " → " << toUpperCase(destination) << "\n";
            cout << "==================================\n";

            if(path.empty() || total==-1){
                cout << "Tidak ada jalur dari " << toUpperCase(source)
                     << " ke " << toUpperCase(destination) << "\n";
            } else {
                cout << "\nShortest Path:\n";
                for(size_t i=0;i<path.size();++i){
                    cout << toUpperCase(path[i]);
                    if(i+1!=path.size()) cout << " -> ";
                }
                cout << "\nTotal Bobot: " << total << "\n";
            }

            printConsoleTable(dist, prev, source);
            printComplexityNote();

            map<string, pair<int,int>> noPos2;
            generateHTML(path, total, source, destination, false, noPos2);
            openBrowser();
            cout << "\n>> HTML updated: index.html\n";
            cout << ">> Please open (or refresh) index.html in your browser to view the result.\n\n";

        } else {
            cout << "Mode tidak dikenal. Coba lagi.\n";
        }
    }

    return 0;
}