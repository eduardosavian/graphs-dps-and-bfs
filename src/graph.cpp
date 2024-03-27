#define USE_CORE_BUILTIN_TYPES
#include "core.hpp"

#include <cstdio>
#include <queue>

#include "small_set.hpp"

using x::dynamic_array, x::slice, x::view, x::pair, x::string;

inline auto arena = x::arena_allocator(x::std_heap_allocator());
constexpr
inline auto default_allocator = arena.as_allocator();

template<typename ListLike, typename CompFunc, typename U>
pair<usize, bool> linear_search(ListLike const& list, U&& val, CompFunc&& fn){
    for(usize i = 0; i < list.size(); i += 1){
        if(fn(list[i], val)){
            return {i, true};
        }
    }
    return {0, false};
}

isize next_unvisited(slice<bool> const& visit_list) {
	for(usize i = 0; i < visit_list.size(); i += 1){
		if(!visit_list[i]){ return i; }
	}
	return -1;
};

isize next_unstudied(slice<i32> const& visit_list) {
	for(usize i = 0; i < visit_list.size(); i += 1){
		if(visit_list[i] != -1){ return i; }
	}
	return -1;
};

template<typename T>
void slice_remove_ordered(slice<T>& s, usize idx){
	if(idx != (s.size() - 1)){
		for(usize i = idx; i < (s.size() - 1); i += 1){
			x::swap(s[i], s[i+1]);
		}
	}

	s = s.sub(0, s.size() - 1);
}

struct graph_node {
	char label = 0;

	graph_node(){}

	graph_node(char label)
	: label{label} {}

    bool operator==(graph_node const& node) const {
        return label == node.label;
    }
    bool operator!=(graph_node const& node) const {
        return !(label == node.label);
    }
};


struct connectivity_matrix {
	using row = slice<bool>;
	using path = slice<graph_node>;
	slice<graph_node> node_map;
	slice<row> rows;

	isize index_of(graph_node node) const {
		for(usize i = 0; i < node_map.size(); i += 1){
			if(node_map[i] == node){ return isize(i); }
		}
		return -1;
	}

	void add_node(graph_node node) {
		for(auto e : node_map){
			if(e == node){ return; }
		}
		auto new_map = make_slice<graph_node>(default_allocator, node_map.size() + 1);
		new_map[new_map.size() - 1] = node;

		// Update map
		x::slice_copy(new_map, node_map);
		node_map = new_map;

		// Update previous rows
		for(usize i = 0; i < rows.size(); i += 1){
			auto row = rows[i];
			auto new_row = make_slice<bool>(default_allocator, row.size() + 1);
			x::slice_copy(new_row, row);
			new_row[new_row.size() - 1] = false;
			rows[i] = new_row;
		}

		// Add new row
		auto new_rows = make_slice<slice<bool>>(default_allocator, rows.size() + 1);
		auto new_row = make_slice<bool>(default_allocator, new_rows.size());
		for(auto& c : new_row){ c = false; }

		x::slice_copy(new_rows, rows);
		new_rows[new_rows.size() - 1] = new_row;
		rows = new_rows;
	}

	void del_node(graph_node node) {
		isize idx = index_of(node);
		if(idx < 0){ return; }

		slice_remove_ordered(rows, idx);
		slice_remove_ordered(node_map, idx);

		for(usize node = 0; node < rows.size(); node += 1){
			// auto& row = rows[node];
			slice_remove_ordered(rows[node], idx);
		}

	}

	bool connected(graph_node a, graph_node b) const {
		auto idx_a = index_of(a);
		auto idx_b = index_of(b);

		if((idx_a < 0) || (idx_b < 0)){
			return false;
		}

		return rows[idx_a][idx_b];
	}

	void connect(graph_node a, graph_node b, bool bidirectional = false){
		set_connection(a, b, true, bidirectional);
	}

	void disconnect(graph_node a, graph_node b, bool bidirectional = false){
		set_connection(a, b, false, bidirectional);
	}

	void set_connection(graph_node a, graph_node b, bool value, bool bidirectional){
		auto idx_a = index_of(a);
		auto idx_b = index_of(b);
		if((idx_a < 0) || (idx_b < 0)){
			return;
		}

		rows[idx_a][idx_b] = value;
		if(bidirectional){
			rows[idx_b][idx_a] = value;
		}
	}

	slice<graph_node> depth_first_search(graph_node start_node) {
		auto visited = x::make_slice<bool>(default_allocator, rows.size());
		auto start = index_of(start_node);
		auto stack = x::stack<usize>(default_allocator);
		auto trail = x::dynamic_array<usize>(default_allocator);

		stack.push(start);

		while(!stack.empty()){
			auto cur = stack.top();
			stack.pop();

			if(visited[cur]){ continue; }

			visited[cur] = true;
			trail.append(cur);

			for(usize adj = 0; adj < rows.size(); adj += 1){
				if(rows[cur][adj]){
					stack.push(adj);
				}
			}
		}

		auto data = label_indices(trail.extract_data());
		for(usize i = 0; i < (data.size() / 2); i += 1){
			x::swap(data[i], data[data.size() - (i+1)]);
		}
		return data;
	}

	slice<graph_node> breadth_first_search(graph_node start_node) {
		auto visited = x::make_slice<bool>(default_allocator, rows.size());
		auto start = index_of(start_node);
		auto queue = std::queue<usize>();
		auto trail = x::dynamic_array<usize>(default_allocator);

		queue.push(start);

		while(!queue.empty()){
			auto cur = queue.front();
			queue.pop();

			if(visited[cur]){ continue; }

			visited[cur] = true;
			trail.append(cur);

			for(usize adj = 0; adj < rows.size(); adj += 1){
				if(rows[cur][adj]){
					queue.push(adj);
				}
			}
		}

		auto data = label_indices(trail.extract_data());
		for(usize i = 0; i < (data.size() / 2); i += 1){
			x::swap(data[i], data[data.size() - (i+1)]);
		}
		return data;
	}

	[[nodiscard]]
	slice<slice<i32>> reachability_matrix() const {
		auto mat = x::make_slice<slice<i32>>(default_allocator, rows.size());
		for(auto& row : mat){
			row = x::make_slice<i32>(default_allocator, rows.size());
		}

		for(usize node = 0; node < rows.size(); node += 1){
			auto clo = transitive_closure(label_index(node));
			for(usize i = 0; i < rows.size(); i += 1){
				mat[node][i] = clo[i].b;
			}
		}

		return mat;
	}

	[[nodiscard]]
	slice<slice<bool>> reachability_matrix_bool() const {
		auto mat = reachability_matrix();
		auto bmat = make_slice<slice<bool>>(default_allocator, mat.size());
		for(auto& b : bmat){
			b = make_slice<bool>(default_allocator, mat.size());
		}

		for(usize i = 0; i < mat.size(); i += 1){
			for(usize j = 0; j < mat.size(); j += 1){
				bmat[i][j] = mat[i][j] > 0;
			}
		}

		return bmat;
	}

	[[nodiscard]]
	slice<slice<graph_node>> strongly_connected_subgraphs() const {
		auto visited = x::make_slice<bool>(default_allocator, rows.size());
		auto reach_mat = reachability_matrix_bool();
		auto subgraphs = dynamic_array<slice<graph_node>>(default_allocator);

		// NOTE: This is only to ensure that the set intersection works properly
		for(usize i = 0; i < reach_mat.size(); i += 1){
			reach_mat[i][i] = true;
		}

		isize node = 0;
		while(node != -1){
			auto row_set = set<usize>(default_allocator);
			auto col_set = set<usize>(default_allocator);

			// Fetch rows
			for(usize i = 0; i < rows.size(); i += 1){
				if(reach_mat[node][i]){
					row_set.add(i);
				}
			}
			// Fetch cols
			for(usize i = 0; i < rows.size(); i += 1){
				if(reach_mat[i][node]){
					col_set.add(i);
				}
			}

			auto inter = row_set.intersect(col_set).extract_data();

			for(auto e : inter){
				visited[e] = true;
			}

			if(inter.size() > 0){
				auto nodes = label_indices(inter);
				subgraphs.append(nodes);
			}

			visited[node] = true;
			node = next_unvisited(visited);
		}

		return subgraphs.extract_data();
	}

	[[nodiscard]]
	slice<pair<graph_node, i32>> transitive_closure(graph_node start_node) const {
		auto visited = x::make_slice<bool>(default_allocator, rows.size());
		auto levels = x::make_slice<i32>(default_allocator, rows.size());

		for(auto& p : levels){ p = -1; }

		auto queue = std::queue<usize>();
		auto start = index_of(start_node);

		queue.push(start);
		levels[start] = 0;

		while(!queue.empty()){
			auto cur = queue.front();
			queue.pop();

			if(visited[cur]){
				continue;
			}

			visited[cur] = true;
			for(usize adj = 0; adj < rows.size(); adj += 1){
				if(rows[cur][adj] && !visited[adj]){
					queue.push(adj);
					levels[adj] = levels[cur] + 1;
				}
			}

		}

		auto res = x::make_slice<pair<graph_node, i32>>(default_allocator, rows.size());
		for(usize i = 0; i < rows.size(); i += 1){
			res[i] = {label_index(i), levels[i]};
		}

		return res;
	}

	[[nodiscard]]
	slice<graph_node> find_path(graph_node start, graph_node target) const {
		auto visited = make_slice<bool>(default_allocator, rows.size());

		x::mem_set(visited.raw_data(), 0, visited.size() * sizeof(bool));

		auto start_idx = index_of(start);
		auto target_idx = index_of(target);
		if((start_idx < 0) || (target_idx < 0)){ return {}; }

		auto path = path_search_rec(start_idx, target_idx, visited);

		auto labeled = make_slice<graph_node>(default_allocator, path.size());
		auto map = x::view(node_map);

		for(usize i = 0; i < labeled.size(); i += 1){
			auto idx = path[path.size() - (i+1)];
			labeled[i] = map[idx];
		}

		return labeled;
	}

	constexpr explicit
	connectivity_matrix(slice<graph_node> nodes) {
		using namespace x;
		node_map = make_slice<graph_node>(default_allocator, nodes.size());
		slice_copy(node_map, nodes);

		rows = make_slice<row>(default_allocator, nodes.size());
		for(auto& row : rows){
			row = make_slice<bool>(default_allocator, nodes.size());
			mem_set(row.raw_data(), 0, row.size() * sizeof(row[0]));
		}
	}

	~connectivity_matrix(){
		for(usize i = 0; i < rows.size(); i += 1){
			x::destroy(default_allocator, rows[i]);
		}

		x::destroy(default_allocator, rows);
		x::destroy(default_allocator, node_map);
	}

	slice<graph_node> label_indices(slice<usize> indexes) const {
		auto labeled = make_slice<graph_node>(default_allocator, indexes.size());

		for(usize i = 0; i < labeled.size(); i += 1){
			auto idx = indexes[indexes.size() - (i+1)];
			labeled[i] = label_index(idx);
		}

		return labeled;
	}

	graph_node label_index(usize index) const {
		auto labeled = graph_node();
		auto map = x::view(node_map);

		labeled = map[index];

		return labeled;
	}

private:
	dynamic_array<usize> path_search_rec(usize cur, usize target, slice<bool>& visited) const {
		visited[cur] = true;

		if(cur == target){
			auto path = dynamic_array<usize>(default_allocator);
			path.append(cur);
			return path;
		}

		auto const& row = rows[cur];
		for(usize neighbor = 0; neighbor < row.size(); neighbor += 1){
			if(row[neighbor] && !visited[neighbor]){
				auto path = path_search_rec(neighbor, target, visited);
				if(!path.empty()){
					path.append(cur);
					return path;
				}
			}
		}

		return dynamic_array<usize>(default_allocator); // No path
	}
};


enum struct ui_operation : i32 	{
	Quit = 0,
	NewGraph,
	AddNode,
	DelNode,
	AddEdge,
	DelEdge,
	Search,
	ClosuresAndReachability,
	Subgraphs,

	Error = -1,
};

constexpr
bool whitespace(char c){
	return (c == ' ') || (c == '\t') || (c == '\r') || (c == '\n');
}

constexpr
bool alphanum(char c){
	return
		((c >= 'a') && (c <= 'z')) || // Lowercase
		((c >= 'A') && (c <= 'Z')) || // Uppercase
		((c >= '0') && (c <= '9'));   // Numeric
}

constexpr
bool valid_node_id(char c){
	return alphanum(c) || (c == '_') || (c == '@') || (c == '$') || (c == '*');
}

usize read_line(slice<char> buf, FILE* stream){
	char c = 0;
	usize buf_idx = 0;
	while(buf_idx < buf.size()){
		c = std::getc(stream);
		if(c == '\n' || c == '\r'){ break; }
		buf[buf_idx] = c;
		buf_idx += 1;
	}
	return buf_idx;
}

struct user_input {
	ui_operation op;
	graph_node a;
	graph_node b;
};

struct ui_context {
	connectivity_matrix mat;
	bool bidirectional;

	ui_context()
		: mat(slice<graph_node>{}), bidirectional(false){}

	void render_matrix(){
		if(mat.rows.size() < 1){
			std::printf("<Empty %s graph>\n\n", bidirectional ? "undirected" : "directed");
			return;
		}

		std::printf("<%s graph>\n", bidirectional ? "Undirected" : "Directed");
		std::printf(" _| ");
		for(auto node : mat.node_map){
			std::printf("%c ", node.label);
		}
		std::printf("\n");

		for(usize i = 0; i < mat.rows.size(); i += 1){
			auto row = mat.rows[i];
			std::printf("%c | ", mat.node_map[i].label);
			for(auto conn : row){
				std::printf("%c ", conn ? '1' : ' ');
			}
			std::printf("\n");
		}

		std::printf("\n");
	}

	slice<char> render_menu(slice<char> line_buf){
		constexpr static auto start_menu =
			"[0] New Graph\n"
			"[1] Add Node\n"
			"[2] Delete Node\n"
			"[3] Add Edge\n"
			"[4] Delete Edge\n"
			"[5] Search\n"
			"[6] Reachability & Closures\n"
			"[7] Strongly Connected Subgraphs\n"
			"[x] Exit\n"
		;

		render_matrix();
		std::puts(start_menu);
		std::printf("> ");
		auto n = read_line(line_buf.sub(0, line_buf.size() - 1), stdin);
		line_buf[n] = 0;
		return line_buf.sub(0, n);
	}

	void render_choose_directionality_menu(slice<char> line_buf){
		while(1){
			std::printf("Is graph directed? [y/n] ");
			auto n = read_line(line_buf.sub(0, line_buf.size() - 1), stdin);
			line_buf[n] = 0;
			auto inp = line_buf.sub(0, n);

			if(inp.size() > 0){
				if(inp[0] == 'y' || inp[0] == 'Y'){
					bidirectional = false;
					break;
				}
				else if(inp[0] == 'n' || inp[0] == 'N'){
					bidirectional = true;
					break;
				}
			}
		}
	}

	void render_new_graph_menu(slice<char> line_buf){
		render_choose_directionality_menu(line_buf);

		std::printf("Enter node labels, e.g: a b c d e\n");
		std::printf("(Node input) > ");

		auto n = read_line(line_buf.sub(0, line_buf.size() - 1), stdin);
		line_buf[n] = 0;
		auto inp = line_buf.sub(0, n);

		// mat = connectivity_matrix()

		auto new_nodes = dynamic_array<graph_node>(default_allocator);

		for(auto c : inp){
			if(valid_node_id(c)){
				// mat.add_node(c);
				new_nodes.append(c);
			}
		}

		mat = connectivity_matrix(new_nodes.extract_data());
	}

	void render_add_nodes_menu(slice<char> line_buf){
		std::printf("Add nodes, e.g: a b x d<Enter> cb <Enter>\n");
		std::printf("Type 'done' when you're finished\n\n");

		while(1){
			std::printf("(Add nodes) > ");
			auto n = read_line(line_buf.sub(0, line_buf.size() - 1), stdin);
			line_buf[n] = 0;
			auto inp = line_buf.sub(0, n);

			if(string(inp) == "done"){
				return;
			}

			for(usize i = 0; i < inp.size(); i += 1){
				auto c = inp[i];
				if(valid_node_id(c)){
					std::printf("+ %c\n", c);
					mat.add_node(c);
				}
			}
		}
	}

	void render_del_nodes_menu(slice<char> line_buf){
		std::printf("Delete nodes, e.g: a b x d<Enter> cb <Enter>\n");
		std::printf("Type 'done' when you're finished\n\n");

		while(1){
			std::printf("(Delete nodes) > ");
			auto n = read_line(line_buf.sub(0, line_buf.size() - 1), stdin);
			line_buf[n] = 0;
			auto inp = line_buf.sub(0, n);

			if(string(inp) == "done"){
				return;
			}

			for(usize i = 0; i < inp.size(); i += 1){
				auto c = inp[i];
				if(valid_node_id(c)){
					std::printf("- %c\n", c);
					mat.del_node(c);
				}
			}
		}
	}

	void render_add_edges_menu(slice<char> line_buf){
		std::printf("Add edges, e.g: a b <Enter> cb <Enter>\n");
		std::printf("Type 'done' when you're finished\n\n");

		while(1){
			std::printf("(Add edges) > ");
			auto n = read_line(line_buf.sub(0, line_buf.size() - 1), stdin);
			line_buf[n] = 0;
			auto inp = line_buf.sub(0, n);

			if(string(inp) == "done"){
				return;
			}

			pair<char, bool> label_a = {0, false};
			pair<char, bool> label_b = {0, false};

			for(usize i = 0; i < inp.size(); i += 1){
				auto c = inp[i];
				if(valid_node_id(c)){
					if(!label_a.b){
						label_a = {c, true};
					}
					else if(!label_b.b) {
						label_b = {c, true};
					}
				}
			}

			if(label_a.b && label_b.b){
				std::printf("%c %s %c\n", label_a.a,  bidirectional ? "<--->" : "--->", label_b.a);
				mat.connect(label_a.a, label_b.a, bidirectional);
			}
			else {
				std::printf("Invalid connection.\n");
			}
		}
	}

	void render_del_edges_menu(slice<char> line_buf){
		std::printf("Delete edges, e.g: a b <Enter> cb <Enter>\n");
		std::printf("Type 'done' when you're finished\n\n");

		while(1){
			std::printf("(Del edges) > ");
			auto n = read_line(line_buf.sub(0, line_buf.size() - 1), stdin);
			line_buf[n] = 0;
			auto inp = line_buf.sub(0, n);

			if(string(inp) == "done"){
				return;
			}

			pair<char, bool> label_a = {0, false};
			pair<char, bool> label_b = {0, false};

			for(usize i = 0; i < inp.size(); i += 1){
				auto c = inp[i];
				if(valid_node_id(c)){
					if(!label_a.b){
						label_a = {c, true};
					}
					else if(!label_b.b) {
						label_b = {c, true};
					}
				}
			}

			if(label_a.b && label_b.b){
				std::printf("%c %s %c\n", label_a.a,  bidirectional ? "<-/->" : "-/->", label_b.a);
				mat.disconnect(label_a.a, label_b.a, bidirectional);
			}
			else {
				std::printf("Invalid connection.\n");
			}
		}
	}

	void render_strongly_connected_subgraphs(){
		auto subgraphs = mat.strongly_connected_subgraphs();

		if((subgraphs.size() == 1) && (subgraphs[0].size() == mat.rows.size())){
			std::printf("Graph is fully connected.\n  ");
			auto graph = subgraphs[0];
			for(auto node : graph){
				std::printf("%c ", node.label);
			}
			std::printf("\n");
		}
		else {
			std::printf("Strongly connected subgraphs:\n");
			for(usize i = 0; i < subgraphs.size(); i += 1){
				std::printf("  Subgraph %zu: ", i);
				auto graph = subgraphs[i];
				for(auto node : graph){
					std::printf("%c ", node.label);
				}
				std::printf("\n");
			}
		}

		std::printf("\n");
	}

	void render_graph_search_menu(slice<char> line_buf){
		std::printf("Starting Node [ ");
		for(auto c : mat.node_map){
			std::printf("%c ", c.label);
		}
		std::printf("]\n");

		graph_node node;
		while(1){
			std::printf("(Search) > ");
			auto n = read_line(line_buf.sub(0, line_buf.size() - 1), stdin);
			line_buf[n] = 0;
			auto inp = line_buf.sub(0, n);

			if(inp.size() > 0){
				node = inp[0];
				break;
			}
		}

		auto bfs = mat.breadth_first_search(node);
		auto dfs = mat.depth_first_search(node);

		std::printf("BFS: ");
		for(auto n : bfs){
			std::printf("%c ", n.label);
		}
		std::printf("\n");

		std::printf("DFS: ");
		for(auto n : dfs){
			std::printf("%c ", n.label);
		}
		std::printf("\n");
	}

	void render_closures_and_rechability_matrix(){
		auto closures = dynamic_array<slice<pair<graph_node, i32>>>(default_allocator);
		std::printf("Transitive closures:\n");
		for(auto node : mat.node_map){
			auto clo = mat.transitive_closure(node);
			closures.append(clo);
			std::printf("%c | ", node.label);
			for(auto [reachable, steps] : clo){
				if(steps > -1){
					std::printf("%c:%d ", reachable.label, steps);
				}
			}
			std::printf("\n");
		}
		std::printf("\n");

		std::printf("Reachability matrix:\n");
		std::printf(" _| ");
		for(auto node : mat.node_map){
			std::printf("%c ", node.label);
		}
		std::printf("\n");

		auto reach_mat = mat.reachability_matrix();
		for(usize i = 0; i < reach_mat.size(); i += 1){
			auto const& row = reach_mat[i];
			std::printf("%c | ", mat.node_map[i].label);
			for(auto n : row){
				if(n < 0){
					std::printf("* ");
				}
				else {
					std::printf("%d ", n);
				}
			}
			std::printf("\n");
		}

		std::printf("\n");
	}

};

slice<graph_node> nodes_from_string(slice<char> str){
	auto nodes = set<graph_node>(default_allocator);
	for(auto c : str){
		if(whitespace(c)){ continue; }

		if(valid_node_id(c)){
			nodes.add(graph_node(c));
		}
		else {
			printf("Invalid node label: %c\n", c);
		}
	}
	return nodes.extract_data();
}

ui_operation input_to_ui_op(slice<char> input){
	using O = ui_operation;
	if(input.size() > 1){
		return O::Error;
	}

	char c = input[0];
	switch (c) {
		case '0': return O::NewGraph; break;
		case '1': return O::AddNode; break;
		case '2': return O::DelNode; break;
		case '3': return O::AddEdge; break;
		case '4': return O::DelEdge; break;
		case '5': return O::Search; break;
		case '6': return O::ClosuresAndReachability; break;
		case '7': return O::Subgraphs; break;
		case 'x': return O::Quit; break;
	}

	return O::Error;
}

int main() {
	auto line_buffer = make_slice<char>(default_allocator, 4 * x::prefix::kibi);
	line_buffer = line_buffer.sub(0, line_buffer.size() - 1);

	auto ui = ui_context();
	bool is_running = true;

	while(is_running){
		auto menu_input = ui.render_menu(line_buffer);

		auto op = input_to_ui_op(menu_input);
		switch (op) {
			using O = ui_operation;

			case O::Quit: {
				is_running = false;
			} break;

			case O::NewGraph: {
				ui.render_new_graph_menu(line_buffer);
			} break;

			case O::AddNode: {
				ui.render_add_nodes_menu(line_buffer);
			} break;

			case O::DelNode: {
				ui.render_del_nodes_menu(line_buffer);
			} break;

			case O::AddEdge: {
				ui.render_add_edges_menu(line_buffer);
			} break;

			case O::DelEdge: {
				ui.render_del_edges_menu(line_buffer);
			} break;

			case O::Search: {
				ui.render_graph_search_menu(line_buffer);
			} break;

			case O::ClosuresAndReachability: {
				ui.render_closures_and_rechability_matrix();
			} break;

			case O::Subgraphs: {
				ui.render_strongly_connected_subgraphs();
			} break;

			case O::Error: {
				std::printf("\n<Unrcognized command: %s>\n", menu_input.raw_data());
			} break;
		}

		// Clear buffer
		x::mem_set(line_buffer.raw_data(), 0, line_buffer.size());
	}
}
