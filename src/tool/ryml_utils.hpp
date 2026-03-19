#ifndef TOOL_RYML_UTILS_HPP
#define TOOL_RYML_UTILS_HPP

#include <cstdio>
#include <ostream>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>

#include <ryml_std.hpp>
#include <ryml.hpp>

namespace ryml_tool {

inline bool defined(const ryml::NodeRef& node) {
	return node.valid() && !node.is_seed();
}

inline ryml::NodeRef child(ryml::NodeRef node, const char* name) {
	if (!defined(node)) {
		return ryml::NodeRef(nullptr);
	}

	ryml::NodeRef result = node.find_child(c4::to_csubstr(name));
	return defined(result) ? result : ryml::NodeRef(nullptr);
}

inline bool has_child(const ryml::NodeRef& node, const char* name) {
	return defined(child(node, name));
}

template <typename T>
T as(const ryml::NodeRef& node) {
	T out{};
	node >> out;
	return out;
}

inline std::string scalar(const ryml::NodeRef& node) {
	if (!defined(node) || !node.has_val()) {
		return {};
	}

	return std::string(node.val().str, node.val().len);
}

inline bool load_tree_from_file(const std::string& path, ryml::Parser& parser, ryml::Tree& tree) {
	FILE* fp = fopen(path.c_str(), "r");
	if (fp == nullptr) {
		return false;
	}

	fseek(fp, 0, SEEK_END);
	size_t size = ftell(fp);
	rewind(fp);

	std::string contents;
	contents.resize(size);

	size_t read = fread(contents.data(), sizeof(char), size, fp);
	fclose(fp);
	contents.resize(read);

	tree.clear();
	parser = {};
	tree = parser.parse_in_arena(c4::to_csubstr(path), c4::to_csubstr(contents));

	return true;
}

namespace emit {
struct BeginMapTag {};
struct EndMapTag {};
struct BeginSeqTag {};
struct EndSeqTag {};
struct KeyTag {};
struct ValueTag {};
struct LiteralTag {};

inline constexpr BeginMapTag BeginMap{};
inline constexpr EndMapTag EndMap{};
inline constexpr BeginSeqTag BeginSeq{};
inline constexpr EndSeqTag EndSeq{};
inline constexpr KeyTag Key{};
inline constexpr ValueTag Value{};
inline constexpr LiteralTag Literal{};
} // namespace emit

class RymlEmitter {
	public:
		RymlEmitter() = default;

		explicit RymlEmitter(std::ostream& out) : out_(&out) {}

		~RymlEmitter() {
			if (out_ != nullptr && initialized_) {
				(*out_) << str();
			}
		}

		RymlEmitter& operator<<(emit::BeginMapTag) {
			begin_container(false);
			return *this;
		}

		RymlEmitter& operator<<(emit::EndMapTag) {
			end_container(false);
			return *this;
		}

		RymlEmitter& operator<<(emit::BeginSeqTag) {
			begin_container(true);
			return *this;
		}

		RymlEmitter& operator<<(emit::EndSeqTag) {
			end_container(true);
			return *this;
		}

		RymlEmitter& operator<<(emit::KeyTag) {
			Frame& frame = current_frame();
			frame.expecting_key = true;
			return *this;
		}

		RymlEmitter& operator<<(emit::ValueTag) {
			return *this;
		}

		RymlEmitter& operator<<(emit::LiteralTag) {
			current_frame().literal_next_value = true;
			return *this;
		}

		RymlEmitter& operator<<(const ryml::NodeRef& node) {
			if (current_frame().expecting_key) {
				current_frame().pending_key = scalar(node);
				current_frame().expecting_key = false;
				current_frame().has_pending_key = true;
				return *this;
			}

			append_node(node);
			return *this;
		}

		RymlEmitter& operator<<(const std::string& value) {
			append_scalar(value);
			return *this;
		}

		RymlEmitter& operator<<(const char* value) {
			append_scalar(std::string(value != nullptr ? value : ""));
			return *this;
		}

		RymlEmitter& operator<<(bool value) {
			append_scalar(value);
			return *this;
		}

		template <typename T>
		std::enable_if_t<std::is_arithmetic_v<T> && !std::is_same_v<T, bool>, RymlEmitter&> operator<<(T value) {
			append_scalar(value);
			return *this;
		}

		const char* c_str() {
			serialized_ = str();
			return serialized_.c_str();
		}

		std::string str() const {
			if (!initialized_) {
				return {};
			}

			return ryml::emitrs<std::string>(tree_);
		}

	private:
		struct Frame {
			size_t node_id = ryml::NONE;
			bool is_seq = false;
			bool expecting_key = false;
			bool has_pending_key = false;
			bool literal_next_value = false;
			std::string pending_key;
		};

		static void copy_node(const ryml::NodeRef& source, ryml::NodeRef target, bool include_key) {
			if (source.is_map()) {
				target = ryml::MAP;
				target |= ryml::_WIP_STYLE_BLOCK;
			} else if (source.is_seq()) {
				target = ryml::SEQ;
				target |= ryml::_WIP_STYLE_BLOCK;
			}

			if (include_key && source.has_key()) {
				std::string key = std::string(source.key().str, source.key().len);
				target << ryml::key(key);
			}

			if (source.has_val()) {
				target << scalar(source);
			}

			if (source.type().val_marked_literal()) {
				target |= ryml::_WIP_VAL_LITERAL;
			}

			for (const ryml::NodeRef child_node : source.children()) {
				ryml::NodeRef child_target = target.append_child();
				copy_node(child_node, child_target, true);
			}
		}

		void begin_container(bool seq) {
			ryml::NodeRef node = append_empty_value();

			if (!initialized_) {
				initialized_ = true;
			}

			node = seq ? ryml::SEQ : ryml::MAP;
			node |= ryml::_WIP_STYLE_BLOCK;

			frames_.push_back(Frame{ node.id(), seq });
		}

		void end_container(bool seq) {
			flush_pending_key();

			if (frames_.empty() || frames_.back().is_seq != seq) {
				throw std::runtime_error("ryml emitter container mismatch");
			}

			frames_.pop_back();
		}

		template <typename T>
		void append_scalar(const T& value) {
			Frame& frame = current_frame();

			if (frame.expecting_key) {
				frame.pending_key = scalar_from(value);
				frame.expecting_key = false;
				frame.has_pending_key = true;
				return;
			}

			ryml::NodeRef node = append_empty_value();
			node << value;

			if (frame.literal_next_value) {
				node |= ryml::_WIP_VAL_LITERAL;
				frame.literal_next_value = false;
			}
		}

		void append_node(const ryml::NodeRef& source) {
			bool include_key = true;
			if (!frames_.empty()) {
				Frame& frame = frames_.back();
				include_key = frame.is_seq || !frame.has_pending_key;
			}

			ryml::NodeRef node = append_empty_value();
			copy_node(source, node, include_key);
		}

		ryml::NodeRef append_empty_value() {
			if (frames_.empty()) {
				if (!initialized_) {
					initialized_ = true;
					return tree_.rootref();
				}

				return tree_.rootref();
			}

			Frame& frame = frames_.back();
			ryml::NodeRef parent(&tree_, frame.node_id);
			ryml::NodeRef child_node = parent.append_child();

			if (!frame.is_seq && frame.has_pending_key) {
				std::string key = frame.pending_key;
				child_node << ryml::key(key);
				frame.has_pending_key = false;
				frame.pending_key.clear();
			}

			if (frame.literal_next_value) {
				frame.literal_next_value = false;
				child_node |= ryml::_WIP_VAL_LITERAL;
			}

			return child_node;
		}

		void flush_pending_key() {
			if (frames_.empty()) {
				return;
			}

			Frame& frame = frames_.back();
			if (frame.is_seq || !frame.has_pending_key) {
				return;
			}

			ryml::NodeRef parent(&tree_, frame.node_id);
			ryml::NodeRef child_node = parent.append_child();
			std::string key = frame.pending_key;
			child_node << ryml::key(key);
			frame.has_pending_key = false;
			frame.pending_key.clear();
			frame.literal_next_value = false;
		}

		Frame& current_frame() {
			if (frames_.empty()) {
				throw std::runtime_error("ryml emitter has no active frame");
			}

			return frames_.back();
		}

		template <typename T>
		static std::string scalar_from(const T& value) {
			if constexpr (std::is_same_v<T, std::string>) {
				return value;
			} else if constexpr (std::is_same_v<T, const char*>) {
				return value != nullptr ? std::string(value) : std::string();
			} else if constexpr (std::is_same_v<T, bool>) {
				return value ? "true" : "false";
			} else {
				return std::to_string(value);
			}
		}

		std::ostream* out_ = nullptr;
		ryml::Tree tree_;
		std::vector<Frame> frames_;
		mutable std::string serialized_;
		bool initialized_ = false;
};

} // namespace ryml_tool

#endif
