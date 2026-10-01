#include "block_index.h"
#include "block_write_plan.h"
#include "file_support.h"
#include <algorithm>
#include <array>
#include <atomic>
#include <charconv>
#include <functional>
#include <map>
#include <portable_file_io.h>
#include <stdexcept>
#include <string_view>
#include <unordered_map>

namespace snfiletools::internal {
namespace {
using Kind = BlockKind;
constexpr std::uint64_t max_disk = std::numeric_limits<std::uint32_t>::max();
std::atomic<std::uint64_t> next_generation{1};

std::uint32_t u32(const char *p) {
    const auto *v = reinterpret_cast<const unsigned char *>(p);
    return std::uint32_t(v[0]) | (std::uint32_t(v[1]) << 8) | (std::uint32_t(v[2]) << 16) | (std::uint32_t(v[3]) << 24);
}
std::string encoded(std::uint64_t value) {
    if (value > max_disk)
        throw std::overflow_error("Block length or offset exceeds u32");
    std::string result(4, '\0');
    for (unsigned i = 0; i != 4; ++i)
        result[i] = static_cast<char>((value >> (8 * i)) & 255);
    return result;
}
std::uint32_t checked_size(std::int64_t size) {
    if (size < 0 || std::uint64_t(size) > max_disk)
        throw std::overflow_error("Block length exceeds u32");
    return static_cast<std::uint32_t>(size);
}
bool metadata_kind(Kind kind) {
    return kind == Kind::index || kind == Kind::file || kind == Kind::page || kind == Kind::layer ||
           kind == Kind::keyword || kind == Kind::title || kind == Kind::link;
}
struct Target {
    bool pointer = false;
    bool unknown = false;
    Kind kind = Kind::opaque;
};
Target target_kind(Kind parent, std::string_view key) {
    auto target = [](Kind kind) { return Target{true, false, kind}; };
    if (parent == Kind::index) {
        if (key == "FILE_FEATURE")
            return target(Kind::file);
        if (key.starts_with("PAGE") && key.size() > 4 &&
            key.substr(4).find_first_not_of("0123456789") == std::string_view::npos)
            return target(Kind::page);
        if (key.starts_with("STYLE_"))
            return target(Kind::opaque);
        if (key.starts_with("KEYWORD_"))
            return target(Kind::keyword);
        if (key.starts_with("TITLE_"))
            return target(Kind::title);
        if (key.starts_with("LINKO_"))
            return target(Kind::link);
        if (key == "COVER_2" || key == "COVER_3" || key == "PDFSTYLELIST")
            return target(Kind::opaque);
    }
    if (parent == Kind::file && key == "HIGHLIGHTINFO")
        return target(Kind::opaque);
    if (parent == Kind::page) {
        if (key == "MAINLAYER" || key == "BGLAYER" || key == "LAYER1" || key == "LAYER2" || key == "LAYER3")
            return target(Kind::layer);
        if (key == "TOTALPATH")
            return target(Kind::trails);
        if (key == "RECOGNTEXT" || key == "RECOGNFILE" || key == "IDTABLE")
            return target(Kind::opaque);
        if (key == "EXTERNALLINKINFO")
            return {true, true, Kind::opaque};
    }
    if (parent == Kind::layer) {
        if (key == "LAYERBITMAP")
            return target(Kind::opaque);
        if (key == "LAYERPATH" || key == "LAYERVECTORGRAPH" || key == "LAYERRECOGN")
            return {true, true, Kind::opaque};
    }
    if ((parent == Kind::keyword && key == "KEYWORDSITE") || (parent == Kind::title && key == "TITLEBITMAP") ||
        (parent == Kind::link && key == "LINKBITMAP"))
        return target(Kind::opaque);
    return {};
}

template <class Resolve> void parse_metadata(IndexedBlock &block, Resolve resolve) {
    block.references.clear();
    const std::string_view text = block.metadata;
    std::size_t p = 0;
    while (p < text.size()) {
        const auto colon = text.find(':', p + 1);
        const auto end = text.find('>', p + 1);
        if (text[p] != '<' || colon == text.npos || end == text.npos || colon <= p + 1 || colon > end ||
            text.substr(p + 1, end - p - 1).find_first_of("<>") != text.npos)
            throw std::invalid_argument("Malformed block metadata");
        const auto key = text.substr(p + 1, colon - p - 1);
        const auto type = target_kind(block.kind, key);
        if (type.pointer) {
            const auto value = text.substr(colon + 1, end - colon - 1);
            std::uint32_t offset = 0;
            const auto [ptr, ec] = std::from_chars(value.data(), value.data() + value.size(), offset);
            if (value.empty() || ec != std::errc{} || ptr != value.data() + value.size())
                throw std::invalid_argument("Invalid decimal block pointer");
            if (offset != 0 && type.unknown)
                throw std::invalid_argument("Unsupported nonzero structural pointer: " + std::string(key));
            block.references.push_back({colon + 1, end, offset == 0 ? no_block : resolve(offset, type.kind, key),
                                        std::string(key)});
        }
        p = end + 1;
    }
}

// A bounds-checked cursor over ONE TOTALPATH, never the whole file.
struct TrailCursor {
    std::string_view bytes;
    std::size_t position, end;
    void skip(std::uint64_t n) {
        if (n > end - position)
            throw std::invalid_argument("Nested field exceeds trail boundary");
        position += static_cast<std::size_t>(n);
    }
    std::uint32_t word() {
        const auto old = position;
        skip(4);
        return u32(bytes.data() + old);
    }
    void collection(std::uint64_t width) {
        const auto count = word();
        skip(count * width);
    }
};

void parse_trail_fields(std::vector<IndexedBlock> &blocks, std::uint32_t parent, std::string_view bytes,
                        std::uint64_t base, std::size_t start, std::size_t end) {
    if (start == end)
        return; // An explicitly emptied block retains its framing.
    TrailCursor c{bytes, start, end};
    c.skip(208);
    for (auto width : {24u, 8u, 2u, 4u, 1u, 8u, 4u})
        c.collection(width);
    c.skip(52);
    c.collection(4);
    c.skip(10);
    const auto groups = c.word();
    if (groups > (c.end - c.position) / 4)
        throw std::invalid_argument("Contour count exceeds trail");
    for (std::uint32_t group = 0; group < groups; ++group)
        c.collection(8);
    c.collection(16);
    c.skip(17);
    c.collection(4);
    c.skip(13);
    auto field = [&](const char *name) {
        const auto prefix = c.position;
        const auto size = c.word();
        c.skip(size);
        blocks.push_back({base + prefix, size, parent, Kind::field, name});
    };
    field("LINK_INFO");
    field("LINK_IMAGE_STRING");
    field("CUSTOM_STRING");
    c.skip(4);
    c.collection(8);
    if (c.position != c.end)
        field("TIMESTAMP");
    if (c.position != c.end)
        c.skip(1);
    if (c.position != c.end)
        throw std::invalid_argument("Unsupported trail suffix");
}

void parse_trails(std::vector<IndexedBlock> &blocks, std::uint32_t parent, std::string_view bytes) {
    if (bytes.empty())
        return;
    const auto base = blocks[parent].offset + 4;
    TrailCursor outer{bytes, 0, bytes.size()};
    const auto count = outer.word();
    if (count > (bytes.size() - 4) / 4)
        throw std::invalid_argument("Trail count exceeds block");
    for (std::uint32_t i = 0; i < count; ++i) {
        const auto prefix = outer.position;
        const auto length = outer.word();
        const auto start = outer.position;
        outer.skip(length);
        const auto id = static_cast<std::uint32_t>(blocks.size());
        blocks.push_back({base + prefix, length, parent, Kind::trail, "TRAIL" + std::to_string(i)});
        parse_trail_fields(blocks, id, bytes, base, start, outer.position);
    }
    if (outer.position != outer.end)
        throw std::invalid_argument("Unmapped trailing bytes in TOTALPATH");
}

} // namespace

BlockIndex::BlockIndex(std::FILE *file) : generation_(next_generation.fetch_add(1)) {
    file_offset_t file_bytes = 0;
    if (file_size(file, &file_bytes) != 0)
        throw io_error("Failed to read file size");
    if (file_bytes < 36 || std::uint64_t(file_bytes) > max_disk)
        throw std::invalid_argument("Unsupported note/mark file size");
    std::array<char, 24> header{};
    block_read_exact(file, 0, header.data(), header.size());
    if ((std::string_view(header.data(), 4) != "note" && std::string_view(header.data(), 4) != "mark") ||
        (std::string_view(header.data() + 4, 20) != "SN_FILE_VER_20260016" &&
         std::string_view(header.data() + 4, 20) != "SN_FILE_VER_20230015"))
        throw std::invalid_argument("Unsupported note/mark header version");
    std::array<char, 8> tail{};
    block_read_exact(file, file_bytes - 8, tail.data(), tail.size());
    if (std::string_view(tail.data(), 4) != "tail")
        throw std::invalid_argument("Missing current file tail");
    std::unordered_map<std::uint64_t, std::uint32_t> offsets;
    auto add = [&](std::uint64_t offset, Kind kind, std::string_view name) {
        if (auto it = offsets.find(offset); it != offsets.end()) {
            if (blocks_[it->second].kind != kind)
                throw std::invalid_argument("Conflicting block pointer kinds");
            return it->second;
        }
        if (offset < 24 || offset > std::uint64_t(file_bytes) - 12)
            throw std::invalid_argument("Block pointer outside file");
        std::array<char, 4> prefix{};
        block_read_exact(file, offset, prefix.data(), prefix.size());
        const auto length = u32(prefix.data());
        if (offset + 4 + length > std::uint64_t(file_bytes) - 8)
            throw std::invalid_argument("Block length outside file");
        const auto id = static_cast<std::uint32_t>(blocks_.size());
        offsets.emplace(offset, id);
        blocks_.push_back({offset, length, no_block, kind, std::string(name)});
        return id;
    };
    root_ = add(u32(tail.data() + 4), Kind::index, "INDEX");
    for (std::size_t id = 0; id < blocks_.size(); ++id) {
        if (!metadata_kind(blocks_[id].kind))
            continue;
        // Resolve may reallocate blocks_; work on a local copy.
        auto block = blocks_[id];
        block.metadata.resize(block.size);
        block_read_exact(file, block.offset + 4, block.metadata.data(), block.metadata.size());
        parse_metadata(block, add);
        blocks_[id] = std::move(block);
    }
    std::vector<std::uint32_t> order;
    for (std::uint32_t id = 0; id < blocks_.size(); ++id)
        order.push_back(id);
    std::sort(order.begin(), order.end(), [&](auto a, auto b) { return blocks_[a].offset < blocks_[b].offset; });
    std::uint64_t end = 24;
    compact_ = true;
    for (auto id : order) {
        const auto &block = blocks_[id];
        if (block.offset < end)
            throw std::invalid_argument("Overlapping live blocks");
        if (block.offset != end)
            compact_ = false;
        end = block.offset + 4 + block.size;
    }
    compact_ =
        compact_ && end == std::uint64_t(file_bytes) - 8 && blocks_[root_].offset + 4 + blocks_[root_].size == end;
    for (auto id : order) {
        if (blocks_[id].kind != Kind::trails)
            continue;
        std::string payload(blocks_[id].size, '\0');
        block_read_exact(file, blocks_[id].offset + 4, payload.data(), payload.size());
        parse_trails(blocks_, id, payload);
    }
    rebuild_traversal();
}

std::vector<BlockInfo> BlockIndex::describe() const {
    std::vector<BlockInfo> result;
    result.reserve(blocks_.size());
    for (std::uint32_t id = 0; id < blocks_.size(); ++id) {
        const auto &block = blocks_[id];
        if (block.live)
            result.push_back({{block.token ? block.token : generation_, id},
                              block.parent == no_block
                                  ? BlockHandle{}
                                  : BlockHandle{blocks_[block.parent].token ? blocks_[block.parent].token : generation_,
                                                block.parent},
                              block.name,
                              block.offset,
                              block.size});
    }
    return result;
}
const IndexedBlock &BlockIndex::get(BlockHandle handle) const {
    if (handle.id >= blocks_.size() || !blocks_[handle.id].live ||
        handle.generation != (blocks_[handle.id].token ? blocks_[handle.id].token : generation_))
        throw std::invalid_argument("Stale block handle or handle from another file");
    return blocks_[handle.id];
}
std::uint64_t BlockIndex::position(const IndexedBlock &block, BlockOrigin origin, std::int64_t offset, bool reading) {
    std::int64_t position = offset;
    if (origin == BlockOrigin::end) {
        if (offset > std::numeric_limits<std::int64_t>::max() - block.size)
            throw std::out_of_range("Block offset overflow");
        position += block.size;
    } else if (origin != BlockOrigin::begin)
        throw std::invalid_argument("Invalid block origin");
    if (position < 0)
        throw std::out_of_range("Read/write starts before block payload");
    if (!reading && std::uint64_t(position) > block.size)
        throw std::out_of_range("Write starts after block payload");
    return static_cast<std::uint64_t>(position);
}

void BlockIndex::rebuild_traversal() {
    children_.assign(blocks_.size(), {});
    physical_.clear();
    traversal_.clear();
    owner_.assign(blocks_.size(), no_block);
    for (std::uint32_t id = 0; id < blocks_.size(); ++id)
        if (blocks_[id].live) {
            if (blocks_[id].parent == no_block)
                physical_.push_back(id);
            else
                children_[blocks_[id].parent].push_back(id);
        }
    auto less = [&](auto a, auto b) { return blocks_[a].offset < blocks_[b].offset; };
    std::sort(physical_.begin(), physical_.end(), less);
    for (auto &children : children_)
        std::sort(children.begin(), children.end(), less);
    std::vector<std::uint32_t> todo(physical_.rbegin(), physical_.rend());
    while (!todo.empty()) {
        auto id = todo.back();
        todo.pop_back();
        traversal_.push_back(id);
        owner_[id] = blocks_[id].parent == no_block ? id : owner_[blocks_[id].parent];
        todo.insert(todo.end(), children_[id].rbegin(), children_[id].rend());
    }
}
BlockInfo BlockIndex::describe(BlockHandle handle) const {
    const auto &b = get(handle);
    auto parent = b.parent == no_block
                      ? BlockHandle{}
                      : BlockHandle{blocks_[b.parent].token ? blocks_[b.parent].token : generation_, b.parent};
    return {handle, parent, b.name, b.offset, b.size};
}
std::vector<BlockInfo> BlockIndex::children(BlockHandle handle) const {
    (void)get(handle);
    std::vector<BlockInfo> result;
    result.reserve(children_[handle.id].size());
    for (auto id : children_[handle.id])
        result.push_back(describe({blocks_[id].token ? blocks_[id].token : generation_, id}));
    return result;
}
std::vector<BlockEdit> BlockIndex::validate(std::vector<BlockEdit> edits) const {
    for (const auto &e : edits) {
        const auto &b = get(e.handle);
        if (e.position > b.size || e.erase > b.size - e.position)
            throw std::out_of_range("Replacement leaves block payload");
        if (e.data.size() > max_disk)
            throw std::overflow_error("Replacement exceeds u32");
    }
    std::erase_if(edits, [](const auto &e) { return e.erase == 0 && e.data.empty(); });
    auto at = [&](const auto &e) { return blocks_[e.handle.id].offset + 4 + e.position; };
    std::stable_sort(edits.begin(), edits.end(), [&](const auto &a, const auto &b) {
        if (at(a) != at(b))
            return at(a) < at(b);
        return a.erase == 0 && b.erase != 0;
    });
    bool leaves_only = std::all_of(edits.begin(), edits.end(), [&](const auto &e) {
        auto kind = blocks_[e.handle.id].kind;
        return kind == Kind::opaque || kind == Kind::field;
    });
    if (leaves_only) {
        std::uint64_t end = 0;
        for (const auto &e : edits) {
            if (at(e) < end)
                throw std::invalid_argument("Overlapping batch edits");
            end = at(e) + e.erase;
        }
        return edits;
    }
    std::uint64_t end = 0;
    std::vector<std::vector<const BlockEdit *>> groups(blocks_.size());
    std::vector<bool> replaced(blocks_.size(), false), retired(blocks_.size(), false);
    for (const auto &e : edits) {
        if (at(e) < end)
            throw std::invalid_argument("Overlapping batch edits");
        end = at(e) + e.erase;
        groups[e.handle.id].push_back(&e);
        const auto &b = blocks_[e.handle.id];
        if (e.position == 0 && e.erase == b.size && (b.kind == Kind::trails || b.kind == Kind::trail))
            replaced[e.handle.id] = true;
    }
    for (auto id : traversal_) {
        auto parent = blocks_[id].parent;
        retired[id] = parent != no_block && (retired[parent] || replaced[parent]);
        if (retired[id] && !groups[id].empty())
            throw std::invalid_argument("Batch edits a replaced parent's descendant");
        if (replaced[id])
            continue; // Global interval checks allow boundary insertions only.
        std::size_t child = 0;
        for (auto e : groups[id]) {
            auto start = at(*e), finish = start + e->erase;
            while (child < children_[id].size() &&
                   blocks_[children_[id][child]].offset + 4 + blocks_[children_[id][child]].size <= start)
                ++child;
            if (child == children_[id].size())
                continue;
            const auto &c = blocks_[children_[id][child]];
            if ((e->erase && start < c.offset + 4 + c.size && finish > c.offset) ||
                (!e->data.empty() && start > c.offset && start < c.offset + 4 + c.size))
                throw std::invalid_argument("Edit intersects a nested block; select its handle");
        }
    }
    return edits;
}
// All phases operate on a candidate. The live index is published only after commit.
// Edits are validated, source-ordered, and remain alive for this planner and plan.
class BlockIndex::ReplacementPlanner {
    const BlockIndex &source;
    std::FILE *file;
    const std::vector<BlockEdit> &edits;
    const std::vector<CreateOperation> &creations;
    const std::vector<AttachOperation> &attachments;
    std::vector<BlockHandle> &created_handles;
    const std::vector<IndexedBlock> &original;
    std::vector<IndexedBlock> &blocks_;
    std::uint32_t root_;
    struct Patch {
        std::uint64_t start, erase;
        std::string value;
        std::span<const char> borrowed{};
    };
    std::vector<std::vector<Patch>> patches;
    std::vector<std::vector<const BlockEdit *>> groups;
    std::vector<std::int64_t> delta;
    std::vector<bool> replaced, retired, live;
    std::vector<std::uint32_t> order;
    std::uint64_t final_size = 0;
    bool rebuilt_children = false;
    std::unordered_map<const CreatedState *, std::uint32_t> pending_ids;

  public:
    ReplacementPlanner(BlockIndex &candidate, const BlockIndex &original_index, const std::vector<BlockEdit> &requests,
                       const std::vector<CreateOperation> &new_blocks, const std::vector<AttachOperation> &links,
                       std::vector<BlockHandle> &results, std::FILE *document)
        : source(original_index), file(document), edits(requests), creations(new_blocks), attachments(links),
          created_handles(results), original(source.blocks_), blocks_(candidate.blocks_),
          root_(candidate.root_), patches(blocks_.size()), groups(blocks_.size()), delta(blocks_.size()),
          replaced(blocks_.size()), retired(blocks_.size()), live(blocks_.size()) {
        for (const auto &creation : creations) {
            const auto kind = creation.kind;
            if (kind == Kind::index || kind == Kind::trail || kind == Kind::field)
                throw std::invalid_argument("Unsupported top-level block kind for creation");
            if (creation.bytes.size() > max_disk)
                throw std::overflow_error("Created block exceeds u32");
            const auto id = static_cast<std::uint32_t>(blocks_.size());
            if (id == no_block)
                throw std::overflow_error("Too many blocks");
            if (!creation.result || !pending_ids.emplace(creation.result.get(), id).second)
                throw std::invalid_argument("Duplicate or missing pending creation result");
            IndexedBlock block;
            block.size = static_cast<std::uint32_t>(creation.bytes.size());
            block.kind = kind;
            block.token = next_generation.fetch_add(1);
            if (metadata_kind(kind))
                block.metadata.assign(creation.bytes.begin(), creation.bytes.end());
            blocks_.push_back(std::move(block));
            created_handles.push_back({blocks_.back().token, id});
        }
        patches.resize(blocks_.size());groups.resize(blocks_.size());delta.resize(blocks_.size());
        replaced.resize(blocks_.size());retired.resize(blocks_.size());live.resize(blocks_.size());
    }
    BlockWritePlan run() {
        group_edits();
        propagate_sizes();
        update_metadata();
        apply_attachments();
        select_live_blocks();
        solve_layout();
        relocate_nested_blocks();
        auto plan = emit_spans();
        rebuild_replaced_children();
        validate_partial_parents(plan);
        return plan;
    }

  private:
    void group_edits() {
        for (const auto &e : edits) {
            auto id = e.handle.id;
            groups[id].push_back(&e);
            delta[id] += static_cast<std::int64_t>(e.data.size()) - static_cast<std::int64_t>(e.erase);
            const auto &b = original[id];
            replaced[id] = replaced[id] ||
                           (e.position == 0 && e.erase == b.size && (b.kind == Kind::trails || b.kind == Kind::trail));
            if (!metadata_kind(b.kind))
                patches[source.owner_[id]].push_back({b.offset + 4 + e.position, e.erase, {}, e.data});
        }
    }

    void propagate_sizes() {
        for (auto id : source.traversal_) {
            auto parent = original[id].parent;
            retired[id] = parent != no_block && (retired[parent] || replaced[parent]);
            if (retired[id])
                blocks_[id].live = false;
        }
        for (auto it = source.traversal_.rbegin(); it != source.traversal_.rend(); ++it) {
            auto id = *it;
            if (retired[id])
                continue;
            blocks_[id].size = checked_size(static_cast<std::int64_t>(original[id].size) + delta[id]);
            auto parent = original[id].parent;
            if (parent != no_block)
                delta[parent] += delta[id];
            if (delta[id] && !metadata_kind(original[id].kind))
                patches[source.owner_[id]].push_back({original[id].offset, 4, encoded(blocks_[id].size), {}});
        }
    }

    void update_metadata() {
        std::unordered_map<std::uint64_t, std::uint32_t> offsets;
        for (auto id : source.physical_)
            offsets.emplace(original[id].offset, id);
        for (auto id : source.physical_)
            if (metadata_kind(original[id].kind) && !groups[id].empty()) {
                std::string text;
                text.reserve(blocks_[id].size);
                std::size_t cursor = 0;
                for (auto e : groups[id]) {
                    text.append(original[id].metadata, cursor, e->position - cursor);
                    text.append(e->data.data() ? e->data.data() : "", e->data.size());
                    cursor = e->position + e->erase;
                }
                text.append(original[id].metadata, cursor, std::string::npos);
                blocks_[id].metadata = std::move(text);
                parse_metadata(blocks_[id], [&](std::uint64_t offset, Kind kind, std::string_view) {
                    auto it = offsets.find(offset);
                    if (it == offsets.end() || blocks_[it->second].kind != kind)
                        throw std::invalid_argument("Replacement pointer must name an existing compatible block");
                    return it->second;
                });
            }
    }

    void apply_attachments() {
        if (creations.empty() && attachments.empty())
            return;
        std::unordered_map<std::uint64_t, std::uint32_t> offsets;
        for (auto id : source.physical_)
            offsets.emplace(original[id].offset, id);
        auto resolve_old = [&](std::uint64_t offset, Kind kind, std::string_view) {
            const auto it = offsets.find(offset);
            if (it == offsets.end() || blocks_[it->second].kind != kind || retired[it->second])
                throw std::invalid_argument("Metadata pointer must name a surviving compatible block");
            return it->second;
        };
        for (std::size_t i = 0; i < creations.size(); ++i) {
            auto &block = blocks_[original.size() + i];
            if (metadata_kind(block.kind))
                parse_metadata(block, [](std::uint64_t, Kind, std::string_view) -> std::uint32_t {
                    throw std::invalid_argument("New metadata pointers must be attached by handle");
                });
        }
        auto id_of = [&](const BlockTarget &target) {
            if (target.pending) {
                const auto it = pending_ids.find(target.pending.get());
                if (it == pending_ids.end() || target.pending->status != CreatedState::pending)
                    throw std::invalid_argument("Pending block belongs to another or cancelled batch");
                return it->second;
            }
            (void)source.get(target.existing);
            if (retired[target.existing.id])
                throw std::invalid_argument("Attachment names a retired block");
            return target.existing.id;
        };
        struct Binding {std::uint32_t parent, target; std::string key;};
        std::vector<Binding> bindings;
        bindings.reserve(attachments.size());
        for (const auto &attachment : attachments) {
            const auto parent = id_of(attachment.parent), target = id_of(attachment.target);
            const auto kind = blocks_[parent].kind;
            if (!metadata_kind(kind))
                throw std::invalid_argument("Attachment parent is not metadata");
            if (parent < original.size() && !groups[parent].empty())
                throw std::invalid_argument("Cannot attach and directly edit the same metadata block in one run");
            const auto expected = target_kind(kind, attachment.key);
            if (!expected.pointer || expected.unknown || expected.kind != blocks_[target].kind)
                throw std::invalid_argument("Attachment key and target kind are incompatible or unsupported");
            if (std::any_of(bindings.begin(), bindings.end(), [&](const auto &b) {
                    return b.parent == parent && b.key == attachment.key;
                }))
                throw std::invalid_argument("Conflicting attachments to the same metadata key");
            bindings.push_back({parent, target, attachment.key});
            auto &text = blocks_[parent].metadata;
            std::size_t found = std::string::npos;
            for (std::size_t p = 0; p < text.size();) {
                const auto colon = text.find(':', p + 1), end = text.find('>', p + 1);
                if (colon == text.npos || end == text.npos)
                    throw std::invalid_argument("Malformed attachment parent metadata");
                if (std::string_view(text).substr(p + 1, colon - p - 1) == attachment.key) {
                    if (found != std::string::npos)
                        throw std::invalid_argument("Duplicate attachment key in metadata");
                    found = p;
                }
                p = end + 1;
            }
            if (found == std::string::npos)
                text += "<" + attachment.key + ":0>";
            else {
                const auto colon = text.find(':', found + 1), end = text.find('>', colon + 1);
                text.replace(colon + 1, end - colon - 1, "0");
            }
        }
        std::vector<bool> changed(blocks_.size());
        for (const auto &binding : bindings)
            changed[binding.parent] = true;
        for (std::uint32_t id = 0; id < blocks_.size(); ++id)
            if (changed[id])
                parse_metadata(blocks_[id], resolve_old);
        for (const auto &binding : bindings) {
            auto &refs = blocks_[binding.parent].references;
            auto it = std::find_if(refs.begin(), refs.end(), [&](const auto &ref) {return ref.key == binding.key;});
            if (it == refs.end())
                throw std::logic_error("Attached key was not parsed");
            it->target = binding.target;
        }
    }

    void select_live_blocks() {
        if (creations.empty() && attachments.empty()) {
            // Preserve the ordinary replacement planner path unchanged.
            std::vector<std::uint32_t> todo{root_};
            while (!todo.empty()) {
                const auto id = todo.back();
                todo.pop_back();
                if (live[id])
                    continue;
                live[id] = true;
                for (const auto &ref : blocks_[id].references)
                    if (ref.target != no_block)
                        todo.push_back(ref.target);
            }
        } else {
            // Match the parser's first-reference naming of shared blocks.
            std::vector<std::uint32_t> todo{root_};
            std::vector<bool> named(blocks_.size());
            named[root_] = true;
            for (std::size_t next = 0; next < todo.size(); ++next) {
                const auto id = todo[next];
                if (live[id])
                    continue;
                live[id] = true;
                for (const auto &ref : blocks_[id].references)
                    if (ref.target != no_block) {
                        if (!named[ref.target] && blocks_[ref.target].parent == no_block) {
                            blocks_[ref.target].name = ref.key;
                            named[ref.target] = true;
                        }
                        todo.push_back(ref.target);
                    }
            }
            for (const auto &binding : attachments)
                if (const auto parent = binding.parent.pending
                                            ? pending_ids.at(binding.parent.pending.get())
                                            : binding.parent.existing.id;
                    !live[parent])
                    throw std::invalid_argument("Attachment parent is not reachable from the document index");
            std::vector<std::uint32_t> indegree(blocks_.size());
            std::size_t top_count = 0;
            for (std::uint32_t id = 0; id < blocks_.size(); ++id)
                if (live[id] && blocks_[id].parent == no_block) {
                    ++top_count;
                    for (const auto &ref : blocks_[id].references)
                        if (ref.target != no_block && live[ref.target])
                            ++indegree[ref.target];
                }
            std::vector<std::uint32_t> roots;
            for (std::uint32_t id = 0; id < blocks_.size(); ++id)
                if (live[id] && blocks_[id].parent == no_block && indegree[id] == 0)
                    roots.push_back(id);
            for (std::size_t next = 0; next < roots.size(); ++next)
                for (const auto &ref : blocks_[roots[next]].references)
                    if (ref.target != no_block && live[ref.target] && --indegree[ref.target] == 0)
                        roots.push_back(ref.target);
            if (roots.size() != top_count)
                throw std::invalid_argument("Structural metadata reference cycle");
        }
        for (auto id : source.traversal_) {
            if (original[id].parent != no_block)
                live[id] = live[source.owner_[id]] && !retired[id];
            blocks_[id].live = live[id];
        }
        for (std::uint32_t id = static_cast<std::uint32_t>(original.size()); id < blocks_.size(); ++id) {
            if (!live[id])
                throw std::invalid_argument("Created block is not attached to the document index");
            blocks_[id].live = true;
        }
        for (auto id : source.physical_)
            if (live[id] && id != root_)
                order.push_back(id);
        for (std::uint32_t id = static_cast<std::uint32_t>(original.size()); id < blocks_.size(); ++id)
            order.push_back(id);
        order.push_back(root_);
    }

    void solve_layout() {
        // Start every decimal pointer at its minimum width. Positions and widths
        // then grow monotonically. At most 9 increases per u32 reference are possible.
        std::vector<std::vector<std::size_t>> widths(blocks_.size());
        std::size_t budget = 1;
        for (auto id : order) {
            widths[id].assign(blocks_[id].references.size(), 1);
            budget += 9 * widths[id].size();
        }
        bool stable = false;
        for (std::size_t pass = 0; pass < budget; ++pass) {
            std::uint64_t cursor = 24;
            for (auto id : order) {
                auto &block = blocks_[id];
                block.offset = cursor;
                if (metadata_kind(block.kind)) {
                    std::int64_t length = static_cast<std::int64_t>(block.metadata.size());
                    for (std::size_t r = 0; r < block.references.size(); ++r)
                        length += static_cast<std::int64_t>(widths[id][r]) -
                                  static_cast<std::int64_t>(block.references[r].end - block.references[r].begin);
                    block.size = checked_size(length);
                }
                cursor += 4 + std::uint64_t(block.size);
            }
            if (cursor + 8 > max_disk)
                throw std::overflow_error("Result exceeds supported u32 file address space");
            final_size = cursor + 8;
            stable = true;
            for (auto id : order) {
                for (std::size_t r = 0; r < blocks_[id].references.size(); ++r) {
                    const auto target = blocks_[id].references[r].target;
                    const auto width = std::to_string(target == no_block ? 0 : blocks_[target].offset).size();
                    if (width < widths[id][r])
                        throw std::logic_error("Nonmonotonic block layout");
                    if (width != widths[id][r]) {
                        stable = false;
                        widths[id][r] = width;
                    }
                }
            }
            if (stable)
                break;
        }
        if (!stable)
            throw std::logic_error("Block layout did not converge");
    }

    void relocate_nested_blocks() {
        // One prefix-sum lookup per surviving nested block, not one shift per edit.
        std::vector<std::uint64_t> ends;
        std::vector<std::int64_t> shifts{0};
        for (const auto &e : edits) {
            const auto &b = original[e.handle.id];
            ends.push_back(b.offset + 4 + e.position + e.erase);
            shifts.push_back(shifts.back() + static_cast<std::int64_t>(e.data.size()) -
                             static_cast<std::int64_t>(e.erase));
        }
        auto shift_at = [&](std::uint64_t at) {
            return shifts[std::upper_bound(ends.begin(), ends.end(), at) - ends.begin()];
        };
        for (auto id : source.traversal_)
            if (live[id] && original[id].parent != no_block) {
                auto owner = source.owner_[id];
                auto relative = static_cast<std::int64_t>(original[id].offset - original[owner].offset) +
                                shift_at(original[id].offset) - shift_at(original[owner].offset);
                blocks_[id].offset = blocks_[owner].offset + relative;
            }
    }

    BlockWritePlan emit_spans() {
        BlockWritePlan plan;
        auto append_literal = [&](std::string value, std::span<const char> borrowed = {}) {
            const auto size = borrowed.empty() ? value.size() : borrowed.size();
            if (size == 0)
                return;
            const auto destination = plan.size;
            plan.size += size;
            if (borrowed.empty() && !plan.spans.empty() && !plan.spans.back().copy &&
                plan.spans.back().borrowed.empty()) {
                plan.spans.back().literal += value;
                plan.spans.back().size += size;
            } else if (borrowed.empty()) {
                plan.spans.push_back(WriteSpan::owned_bytes(destination, std::move(value)));
            } else {
                plan.spans.push_back(WriteSpan::borrowed_bytes(destination, borrowed));
            }
        };
        auto copy = [&](std::uint64_t source, std::uint64_t size) {
            if (size == 0)
                return;
            if (!plan.spans.empty() && plan.spans.back().copy &&
                plan.spans.back().source + plan.spans.back().size == source)
                plan.spans.back().size += size;
            else
                plan.spans.push_back(WriteSpan::copy_from(source, plan.size, size));
            plan.size += size;
        };
        copy(0, 24);
        for (auto id : order) {
            auto &block = blocks_[id];
            if (plan.size != block.offset)
                throw std::logic_error("Layout and output disagree");
            if (metadata_kind(block.kind)) {
                std::string rendered;
                std::size_t cursor = 0;
                for (auto &ref : block.references) {
                    rendered.append(block.metadata, cursor, ref.begin - cursor);
                    cursor = ref.end;
                    ref.begin = rendered.size();
                    rendered += std::to_string(ref.target == no_block ? 0 : blocks_[ref.target].offset);
                    ref.end = rendered.size();
                }
                rendered.append(block.metadata, cursor, std::string::npos);
                if (rendered.size() != block.size)
                    throw std::logic_error("Metadata size disagrees with layout");
                if (id < original.size() && id != root_ && rendered == original[id].metadata &&
                    block.size == original[id].size)
                    copy(original[id].offset, 4 + std::uint64_t(original[id].size));
                else
                    append_literal(encoded(block.size) + rendered);
                block.metadata = std::move(rendered);
                continue;
            }
            if (id >= original.size()) {
                const auto &bytes = creations[id - original.size()].bytes;
                append_literal(encoded(block.size));
                append_literal({}, bytes);
                continue;
            }
            const auto &old = original[id];
            auto &block_patches = patches[id];
            if (block_patches.empty()) {
                copy(old.offset, 4 + std::uint64_t(old.size));
                continue;
            }
            std::stable_sort(block_patches.begin(), block_patches.end(), [](const auto &a, const auto &b) {
                if (a.start != b.start)
                    return a.start < b.start;
                return a.erase == 0 && b.erase != 0;
            });
            auto cursor = old.offset;
            for (auto &patch : block_patches) {
                if (patch.start < cursor)
                    throw std::logic_error("Overlapping structural patches");
                copy(cursor, patch.start - cursor);
                append_literal(std::move(patch.value), patch.borrowed);
                cursor = patch.start + patch.erase;
            }
            copy(cursor, old.offset + 4 + old.size - cursor);
        }
        append_literal("tail" + encoded(blocks_[root_].offset));
        if (plan.size != final_size)
            throw std::logic_error("Final size disagrees with layout");
        return plan;
    }

    void validate_partial_parents(const BlockWritePlan &plan) const {
        // Only direct edits of structural parents need reparsing. Leaf edits
        // have library-owned length updates and keep the existing fast path.
        std::vector<std::vector<std::uint32_t>> children;
        for (auto id : source.traversal_) {
            const auto kind = original[id].kind;
            if (!live[id] || replaced[id] || groups[id].empty() ||
                (kind != Kind::trail && kind != Kind::trails))
                continue;
            const auto &block = blocks_[id];
            std::string payload(block.size, '\0');
            const auto begin = block.offset + 4, end = begin + block.size;
            // Spans are emitted in destination order. Start at the first one
            // that overlaps this parent rather than revisiting the whole plan.
            auto span = std::lower_bound(plan.spans.begin(), plan.spans.end(), begin,
                                         [](const WriteSpan &item, std::uint64_t at) {
                                             return item.destination + item.size <= at;
                                         });
            for (; span != plan.spans.end() && span->destination < end; ++span) {
                const auto first = std::max(begin, span->destination);
                const auto last = std::min(end, span->destination + span->size);
                if (first >= last)
                    continue;
                const auto relative = first - span->destination;
                if (span->copy) {
                    if (file_read_exact_at(file, static_cast<file_offset_t>(span->source + relative),
                                           payload.data() + first - begin, last - first))
                        throw io_error("Read structural parent for validation");
                } else {
                    const auto bytes = span->bytes();
                    std::copy_n(bytes.data() + relative, last - first, payload.data() + first - begin);
                }
            }
            std::vector<IndexedBlock> parsed{block};
            if (kind == Kind::trails)
                parse_trails(parsed, 0, payload);
            else
                parse_trail_fields(parsed, 0, payload, begin, 0, payload.size());
            // Compare framing, not payload semantics or values. Existing handles
            // must still designate exactly the children described by the bytes.
            using Shape = std::tuple<std::uint64_t, std::uint32_t, Kind, std::string, std::uint64_t>;
            std::vector<Shape> expected, actual;
            for (std::size_t child = 1; child < parsed.size(); ++child) {
                const auto &b = parsed[child];
                actual.emplace_back(b.offset, b.size, b.kind, b.name, parsed[b.parent].offset);
            }
            // Original child links remain valid until a replacement retires
            // and reuses IDs. Build candidate links only for that case.
            if (rebuilt_children && children.empty()) {
                children.resize(blocks_.size());
                for (std::uint32_t child = 0; child < blocks_.size(); ++child) {
                    const auto &b = blocks_[child];
                    if (b.live && b.parent != no_block)
                        children[b.parent].push_back(child);
                }
            }
            const auto &links = rebuilt_children ? children : source.children_;
            std::vector<std::uint32_t> pending(links[id].begin(), links[id].end());
            while (!pending.empty()) {
                const auto child = pending.back();
                pending.pop_back();
                const auto &b = blocks_[child];
                if (!b.live)
                    continue;
                expected.emplace_back(b.offset, b.size, b.kind, b.name, blocks_[b.parent].offset);
                pending.insert(pending.end(), links[child].begin(), links[child].end());
            }
            std::sort(expected.begin(), expected.end());
            std::sort(actual.begin(), actual.end());
            if (actual != expected)
                throw std::invalid_argument("Partial edit changes nested structure; replace the entire parent");
        }
    }

    void rebuild_replaced_children() {
        std::vector<std::uint32_t> vacant;
        for (std::uint32_t id = 0; id < blocks_.size(); ++id)
            if (!blocks_[id].live)
                vacant.push_back(id);
        auto install = [&](std::uint32_t id, Kind kind, std::string_view view) {
            rebuilt_children = true;
            std::vector<IndexedBlock> subtree{blocks_[id]};
            if (kind == Kind::trails)
                parse_trails(subtree, 0, view);
            else
                parse_trail_fields(subtree, 0, view, subtree[0].offset + 4, 0, view.size());
            std::vector<std::uint32_t> remap(subtree.size());
            remap[0] = id;
            for (std::size_t i = 1; i < subtree.size(); ++i) {
                auto node = std::move(subtree[i]);
                node.parent = remap[node.parent];
                node.token = next_generation.fetch_add(1);
                if (vacant.empty()) {
                    remap[i] = static_cast<std::uint32_t>(blocks_.size());
                    blocks_.push_back(std::move(node));
                } else {
                    remap[i] = vacant.back();
                    vacant.pop_back();
                    blocks_[remap[i]] = std::move(node);
                }
            }
        };
        for (auto id : source.traversal_)
            if (replaced[id] && live[id]) {
                std::string payload;
                std::string_view view;
                // Queued bytes remain owned until this synchronous run finishes.
                // Only concatenated replacements need another payload allocation.
                if (groups[id].size() == 1) {
                    const auto data = groups[id].front()->data;
                    if (!data.empty())
                        view = std::string_view(data.data(), data.size());
                } else {
                    payload.reserve(blocks_[id].size);
                    for (const auto *edit : groups[id])
                        if (!edit->data.empty())
                            payload.append(edit->data.data(), edit->data.size());
                    view = payload;
                }
                install(id, original[id].kind, view);
            }
        for (std::size_t i = 0; i < creations.size(); ++i)
            if (creations[i].kind == Kind::trails) {
                const auto &bytes = creations[i].bytes;
                install(static_cast<std::uint32_t>(original.size() + i), Kind::trails,
                        std::string_view(bytes.data(), bytes.size()));
            }
    }
};

BlockWritePlan BlockIndex::replace_many(const BlockIndex &source, const std::vector<BlockEdit> &edits,
                                        const std::vector<CreateOperation> &creations,
                                        const std::vector<AttachOperation> &attachments,
                                        std::vector<BlockHandle> &created_handles, std::FILE *file) {
    auto plan = ReplacementPlanner(*this, source, edits, creations, attachments, created_handles, file).run();
    rebuild_traversal();
    compact_ = true;
    return plan;
}

} // namespace snfiletools::internal
