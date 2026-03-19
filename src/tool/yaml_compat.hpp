#ifndef YAML_COMPAT_HPP
#define YAML_COMPAT_HPP

#include <algorithm>
#include <cctype>
#include <fstream>
#include <iterator>
#include <memory>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include <c4/yml/emit.hpp>
#include <ryml_std.hpp>
#include <ryml.hpp>

namespace YAML {

struct Mark {
	int line = 0;
	int column = 0;
};

class Exception : public std::runtime_error {
public:
	std::string msg;
	Mark mark;

	Exception(std::string message, Mark location)
	: std::runtime_error(message), msg(std::move(message)), mark(location) {
	}
};

namespace detail {

struct ParsedDocument {
	ryml::Tree tree;
};

inline std::string toString(c4::csubstr value) {
	if (value.str == nullptr || value.len == 0) {
		return "";
	}

	return std::string(value.str, value.len);
}

inline std::string trimCopy(std::string value) {
	auto not_space = [](unsigned char ch) {
		return !std::isspace(ch);
	};

	value.erase(value.begin(), std::find_if(value.begin(), value.end(), not_space));
	value.erase(std::find_if(value.rbegin(), value.rend(), not_space).base(), value.end());
	return value;
}

inline std::string lowerCopy(std::string value) {
	std::transform(value.begin(), value.end(), value.begin(), [](unsigned char ch) {
		return static_cast<char>(std::tolower(ch));
	});
	return value;
}

inline std::string readFile(const std::string& path) {
	std::ifstream file(path, std::ios::binary);

	if (!file.is_open()) {
		throw Exception("unable to open file: " + path, {});
	}

	return std::string(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
}

inline void throwParseError(const char* msg, size_t msg_len, ryml::Location location, void* user_data) {
	(void)user_data;

	throw Exception(std::string(msg, msg_len), { static_cast<int>(location.line), static_cast<int>(location.col) });
}

inline bool parseBool(const std::string& value) {
	const std::string lowered = lowerCopy(trimCopy(value));

	if (lowered == "true" || lowered == "yes" || lowered == "on" || lowered == "1") {
		return true;
	}

	if (lowered == "false" || lowered == "no" || lowered == "off" || lowered == "0") {
		return false;
	}

	throw Exception("invalid boolean value: " + value, {});
}

template <typename T>
inline T parseScalar(const std::string& value) {
	if constexpr (std::is_same_v<T, std::string>) {
		return value;
	} else if constexpr (std::is_same_v<T, bool>) {
		return parseBool(value);
	} else if constexpr (std::is_floating_point_v<T>) {
		return static_cast<T>(std::stod(value));
	} else if constexpr (std::is_integral_v<T> && std::is_signed_v<T>) {
		return static_cast<T>(std::stoll(value));
	} else if constexpr (std::is_integral_v<T> && std::is_unsigned_v<T>) {
		return static_cast<T>(std::stoull(value));
	} else {
		static_assert(std::is_same_v<T, void>, "Unsupported YAML scalar conversion");
	}
}

inline bool isReservedPlainScalar(const std::string& value) {
	const std::string lowered = lowerCopy(value);

	return lowered == "null" || lowered == "~" || lowered == "true" || lowered == "false" ||
		lowered == "yes" || lowered == "no" || lowered == "on" || lowered == "off";
}

inline bool needsQuotes(const std::string& value) {
	if (value.empty()) {
		return true;
	}

	if (std::isspace(static_cast<unsigned char>(value.front())) || std::isspace(static_cast<unsigned char>(value.back()))) {
		return true;
	}

	if (isReservedPlainScalar(value)) {
		return false;
	}

	switch (value.front()) {
		case '-':
		case '?':
		case ':':
		case ',':
		case '[':
		case ']':
		case '{':
		case '}':
		case '#':
		case '&':
		case '*':
		case '!':
		case '|':
		case '>':
		case '\'':
		case '"':
		case '%':
		case '@':
		case '`':
			return true;
		default:
			break;
	}

	if (value.find(": ") != std::string::npos || value.find(" #") != std::string::npos ||
		value.find('\t') != std::string::npos || value.find('\r') != std::string::npos ||
		value.find('[') != std::string::npos || value.find(']') != std::string::npos ||
		value.find('{') != std::string::npos || value.find('}') != std::string::npos ||
		value.find(',') != std::string::npos) {
		return true;
	}

	return false;
}

inline std::string quoteScalar(const std::string& value) {
	std::string quoted = "\"";

	for (char ch : value) {
		switch (ch) {
			case '\\':
				quoted += "\\\\";
				break;
			case '"':
				quoted += "\\\"";
				break;
			case '\n':
				quoted += "\\n";
				break;
			case '\t':
				quoted += "\\t";
				break;
			case '\r':
				break;
			default:
				quoted += ch;
				break;
		}
	}

	quoted += "\"";
	return quoted;
}

inline std::vector<std::string> splitLines(const std::string& value) {
	std::vector<std::string> lines;
	std::string current;

	for (char ch : value) {
		if (ch == '\r') {
			continue;
		}

		if (ch == '\n') {
			lines.push_back(current);
			current.clear();
			continue;
		}

		current += ch;
	}

	lines.push_back(current);
	return lines;
}

} // namespace detail

class Node {
public:
	class iterator;

	Node() : node_(nullptr) {
	}

	Node(std::shared_ptr<detail::ParsedDocument> doc, const ryml::NodeRef& node)
	: doc_(std::move(doc)), node_(node) {
	}

	Node(const Node&) = default;
	Node(Node&&) noexcept = default;
	Node& operator=(const Node&) = default;
	Node& operator=(Node&&) noexcept = default;

	template <typename T, typename = std::enable_if_t<!std::is_same_v<std::decay_t<T>, Node>>>
	Node& operator=(T&& value) {
		if (!doc_ || !node_.valid()) {
			return *this;
		}

		assign(std::forward<T>(value));
		return *this;
	}

	bool IsDefined() const {
		return doc_ != nullptr && node_.valid() && !node_.is_seed();
	}

	explicit operator bool() const {
		return this->IsDefined();
	}

	void reset() {
		doc_.reset();
		node_ = ryml::NodeRef(nullptr);
	}

	Node operator[](const char* key) {
		if (!doc_ || !node_.valid()) {
			return {};
		}

		return { doc_, node_[c4::to_csubstr(key)] };
	}

	Node operator[](const std::string& key) {
		return (*this)[key.c_str()];
	}

	Node operator[](size_t index) {
		if (!doc_ || !node_.valid()) {
			return {};
		}

		return { doc_, node_[index] };
	}

	Node operator[](const char* key) const {
		if (!this->IsDefined() || !node_.has_child(c4::to_csubstr(key))) {
			return {};
		}

		return { doc_, node_.find_child(c4::to_csubstr(key)) };
	}

	Node operator[](const std::string& key) const {
		return (*this)[key.c_str()];
	}

	Node operator[](size_t index) const {
		if (!this->IsDefined() || index >= node_.num_children()) {
			return {};
		}

		return { doc_, node_.child(index) };
	}

	template <typename T>
	T as() const {
		if (!this->IsDefined()) {
			throw Exception("attempted to read an undefined node", {});
		}

		return detail::parseScalar<T>(detail::toString(node_.val()));
	}

	bool isContainer() const {
		return this->IsDefined() && node_.is_container();
	}

	bool isSeq() const {
		return this->IsDefined() && node_.is_seq();
	}

	bool isLiteral() const {
		return this->IsDefined() && node_.type().val_marked_literal();
	}

	bool hasNewline() const {
		if (!this->IsDefined() || !node_.has_val()) {
			return false;
		}

		const c4::csubstr value = node_.val();
		return value.str != nullptr && std::find(value.begin(), value.end(), '\n') != value.end();
	}

	std::string scalar() const {
		if (!this->IsDefined() || !node_.has_val()) {
			return "";
		}

		return detail::toString(node_.val());
	}

	std::string emitYaml() const {
		if (!this->IsDefined()) {
			return "";
		}

		return ryml::emitrs<std::string>(node_);
	}

	void remove(const std::string& key) {
		if (!this->IsDefined()) {
			return;
		}

		if (node_.has_child(c4::to_csubstr(key))) {
			node_.remove_child(c4::to_csubstr(key));
		}
	}

	template <typename T>
	void force_insert(const std::string& key, T&& value) {
		this->remove(key);
		(*this)[key] = std::forward<T>(value);
	}

	iterator begin() const;
	iterator end() const;

private:
	template <typename T>
	void assign(T&& value) {
		if constexpr (std::is_same_v<std::decay_t<T>, bool>) {
			node_ << value;
		} else if constexpr (std::is_arithmetic_v<std::decay_t<T>>) {
			node_ << value;
		} else if constexpr (std::is_same_v<std::decay_t<T>, std::string>) {
			node_ << value;
		} else if constexpr (std::is_convertible_v<T, const char*>) {
			const char* literal = value;
			node_ << std::string(literal != nullptr ? literal : "");
		} else {
			static_assert(std::is_same_v<std::decay_t<T>, void>, "Unsupported YAML assignment type");
		}
	}

	std::shared_ptr<detail::ParsedDocument> doc_;
	ryml::NodeRef node_;
};

class Node::iterator {
public:
	using iterator_category = std::forward_iterator_tag;
	using value_type = Node;
	using difference_type = std::ptrdiff_t;

	iterator() = default;

	iterator(std::shared_ptr<detail::ParsedDocument> doc, size_t id)
	: doc_(std::move(doc)), id_(id) {
	}

	value_type operator*() const {
		if (!doc_ || id_ == ryml::NONE) {
			return {};
		}

		return { doc_, ryml::NodeRef(&doc_->tree, id_) };
	}

	iterator& operator++() {
		if (doc_ && id_ != ryml::NONE) {
			id_ = doc_->tree.next_sibling(id_);
		}

		return *this;
	}

	bool operator==(const iterator& other) const {
		return doc_.get() == other.doc_.get() && id_ == other.id_;
	}

	bool operator!=(const iterator& other) const {
		return !(*this == other);
	}

private:
	std::shared_ptr<detail::ParsedDocument> doc_;
	size_t id_ = ryml::NONE;
};

inline Node::iterator Node::begin() const {
	if (!this->IsDefined() || !node_.has_children()) {
		return this->end();
	}

	return { doc_, node_.first_child().id() };
}

inline Node::iterator Node::end() const {
	return { doc_, ryml::NONE };
}

inline Node LoadFile(const std::string& path) {
	const std::string contents = detail::readFile(path);
	auto document = std::make_shared<detail::ParsedDocument>();

	const ryml::Callbacks previous = ryml::get_callbacks();
	ryml::Callbacks callbacks = previous;
	callbacks.m_error = detail::throwParseError;
	ryml::set_callbacks(callbacks);

	try {
		document->tree = ryml::parse_in_arena(c4::to_csubstr(path), c4::to_csubstr(contents));
	} catch (...) {
		ryml::set_callbacks(previous);
		throw;
	}

	ryml::set_callbacks(previous);
	return { document, document->tree.rootref() };
}

struct _BeginMapTag {};
struct _EndMapTag {};
struct _BeginSeqTag {};
struct _EndSeqTag {};
struct _KeyTag {};
struct _ValueTag {};
struct _LiteralTag {};

inline constexpr _BeginMapTag BeginMap{};
inline constexpr _EndMapTag EndMap{};
inline constexpr _BeginSeqTag BeginSeq{};
inline constexpr _EndSeqTag EndSeq{};
inline constexpr _KeyTag Key{};
inline constexpr _ValueTag Value{};
inline constexpr _LiteralTag Literal{};

class Emitter {
public:
	Emitter() = default;

	explicit Emitter(std::ostream& output)
	: output_(&output) {
	}

	Emitter(const Emitter&) = delete;
	Emitter& operator=(const Emitter&) = delete;
	Emitter(Emitter&&) noexcept = default;
	Emitter& operator=(Emitter&&) noexcept = default;

	const char* c_str() const {
		return buffer_.c_str();
	}

	Emitter& operator<<(const _BeginMapTag&) {
		startContainer(ContainerType::Map);
		return *this;
	}

	Emitter& operator<<(const _EndMapTag&) {
		endContainer(ContainerType::Map);
		return *this;
	}

	Emitter& operator<<(const _BeginSeqTag&) {
		startContainer(ContainerType::Seq);
		return *this;
	}

	Emitter& operator<<(const _EndSeqTag&) {
		endContainer(ContainerType::Seq);
		return *this;
	}

	Emitter& operator<<(const _KeyTag&) {
		if (!stack_.empty() && stack_.back().type == ContainerType::Map) {
			stack_.back().expecting_key = true;
		}
		return *this;
	}

	Emitter& operator<<(const _ValueTag&) {
		return *this;
	}

	Emitter& operator<<(const _LiteralTag&) {
		literal_next_ = true;
		return *this;
	}

	Emitter& operator<<(const Node& node) {
		if (!node.IsDefined()) {
			return *this;
		}

		if (node.isContainer()) {
			writeRenderedNode(node.emitYaml(), node.isSeq());
			return *this;
		}

		writeScalar(node.scalar(), literal_next_ || node.isLiteral() || node.hasNewline(), true);
		return *this;
	}

	Emitter& operator<<(const std::string& value) {
		writeScalar(value, literal_next_, false);
		return *this;
	}

	Emitter& operator<<(const char* value) {
		writeScalar(value != nullptr ? std::string(value) : std::string(), literal_next_, false);
		return *this;
	}

	template <typename T, typename = std::enable_if_t<std::is_arithmetic_v<T> && !std::is_same_v<T, bool>>>
	Emitter& operator<<(T value) {
		std::ostringstream stream;
		stream << value;
		writeScalar(stream.str(), false, true);
		return *this;
	}

	Emitter& operator<<(bool value) {
		writeScalar(value ? "true" : "false", false, true);
		return *this;
	}

private:
	enum class ContainerType {
		Map,
		Seq,
	};

	struct Context {
		ContainerType type;
		int indent;
		bool first = true;
		bool inline_first = false;
		bool expecting_key = false;
		std::string pending_key;
	};

	void append(const std::string& text) {
		if (output_ != nullptr) {
			(*output_) << text;
		} else {
			buffer_ += text;
		}
	}

	void append(char ch) {
		if (output_ != nullptr) {
			(*output_) << ch;
		} else {
			buffer_ += ch;
		}
	}

	void appendIndent(int indent) {
		if (indent > 0) {
			append(std::string(static_cast<size_t>(indent), ' '));
		}
	}

	void appendMapKeyPrefix(Context& context) {
		if (context.inline_first && context.first) {
			return;
		}

		appendIndent(context.indent);
	}

	void appendRenderedLines(const std::vector<std::string>& lines, int indent) {
		for (const std::string& line : lines) {
			appendIndent(indent);
			append(line);
			append('\n');
		}
	}

	void startContainer(ContainerType type) {
		literal_next_ = false;

		if (stack_.empty()) {
			stack_.push_back({ type, 0 });
			return;
		}

		Context& parent = stack_.back();

		if (parent.type == ContainerType::Map && !parent.pending_key.empty()) {
			appendMapKeyPrefix(parent);
			append(parent.pending_key);
			append(':');
			append('\n');
			parent.pending_key.clear();
			parent.first = false;
			stack_.push_back({ type, parent.indent + 2 });
			return;
		}

		if (parent.type == ContainerType::Seq) {
			appendIndent(parent.indent);

			if (type == ContainerType::Map) {
				append("- ");
				stack_.push_back({ type, parent.indent + 2, true, true });
			} else {
				append('-');
				append('\n');
				stack_.push_back({ type, parent.indent + 2 });
			}
			return;
		}

		stack_.push_back({ type, 0 });
	}

	void endContainer(ContainerType type) {
		literal_next_ = false;

		if (!stack_.empty() && stack_.back().type == type) {
			stack_.pop_back();
		}
	}

	void writeRenderedNode(const std::string& rendered, bool is_seq_node) {
		literal_next_ = false;

		const std::vector<std::string> lines = detail::splitLines(rendered);

		if (stack_.empty()) {
			append(rendered);
			return;
		}

		Context& context = stack_.back();

		if (context.type == ContainerType::Map && !context.pending_key.empty()) {
			appendMapKeyPrefix(context);
			append(context.pending_key);
			append(':');
			append('\n');
			appendRenderedLines(lines, context.indent + 2);
			context.pending_key.clear();
			context.first = false;
			return;
		}

		if (context.type == ContainerType::Seq) {
			appendIndent(context.indent);

			if (is_seq_node) {
				append('-');
				append('\n');
				appendRenderedLines(lines, context.indent + 2);
				return;
			}

			append("- ");
			append(lines.front());
			append('\n');

			for (size_t i = 1; i < lines.size(); ++i) {
				appendIndent(context.indent + 2);
				append(lines[i]);
				append('\n');
			}
		}
	}

	void writeLiteralValue(Context& context, const std::string& value) {
		const std::vector<std::string> lines = detail::splitLines(value);

		append("|-");
		append('\n');

		for (const std::string& line : lines) {
			appendIndent(context.indent + 2);
			append(line);
			append('\n');
		}
	}

	void writeMapScalar(Context& context, const std::string& rendered, bool literal) {
		appendMapKeyPrefix(context);
		append(context.pending_key);
		append(": ");

		if (literal) {
			writeLiteralValue(context, rendered);
		} else {
			append(rendered);
			append('\n');
		}

		context.pending_key.clear();
		context.first = false;
	}

	void writeSeqScalar(Context& context, const std::string& rendered, bool literal) {
		appendIndent(context.indent);
		append("- ");

		if (literal) {
			writeLiteralValue(context, rendered);
		} else {
			append(rendered);
			append('\n');
		}
	}

	void writeScalar(const std::string& value, bool literal, bool already_rendered) {
		literal_next_ = false;

		std::string rendered = value;

		if (!already_rendered && !literal && detail::needsQuotes(value)) {
			rendered = detail::quoteScalar(value);
		}

		if (stack_.empty()) {
			append(rendered);
			return;
		}

		Context& context = stack_.back();

		if (context.type == ContainerType::Map) {
			if (context.expecting_key) {
				context.pending_key = value;
				context.expecting_key = false;
				return;
			}

			if (!context.pending_key.empty()) {
				writeMapScalar(context, rendered, literal);
			}
			return;
		}

		writeSeqScalar(context, rendered, literal);
	}

	std::ostream* output_ = nullptr;
	std::string buffer_;
	std::vector<Context> stack_;
	bool literal_next_ = false;
};

} // namespace YAML

#endif // YAML_COMPAT_HPP
