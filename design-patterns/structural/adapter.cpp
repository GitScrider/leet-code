/*
 * Pattern: Adapter (Structural)
 *
 * Intent:
 *   Convert the interface of an existing class into another interface that
 *   clients expect, letting classes work together that otherwise could not
 *   because of incompatible interfaces.
 *
 * Problem / When to use it:
 *   - You want to reuse an existing class whose interface does not match the
 *     one your client code requires.
 *   - You are integrating a third-party or legacy component you cannot modify.
 *   - You need a stable target interface while the concrete data source varies.
 *
 * Real-world analogy:
 *   A travel power-plug adapter lets a device with one plug shape draw current
 *   from a wall socket of a different shape, without rewiring either side.
 *
 * Participants:
 *   - Target (JSONDataSource): interface the client depends on.
 *   - Adaptee (LegacyXmlProvider): existing class with an incompatible API.
 *   - Adapter (XmlToJsonAdapter): implements Target, wraps an Adaptee, and
 *     translates each call.
 *   - Client (printReport): works only against the Target interface.
 *
 * Trade-offs:
 *   Pros:
 *     - Reuses existing/legacy code without changing it (Open/Closed).
 *     - Isolates interface-conversion logic in one place.
 *   Cons:
 *     - Adds an extra layer of indirection and one more class.
 *     - Deeply mismatched interfaces can make the adapter itself complex.
 */

#include <iostream>
#include <memory>
#include <string>
#include <vector>

// --- Target: the modern interface the client expects ("round hole") ---
class JSONDataSource {
public:
    virtual ~JSONDataSource() = default;
    // Returns records already shaped as JSON-like strings.
    virtual std::vector<std::string> fetchAsJson() const = 0;
};

// --- Adaptee: an existing, incompatible class we must reuse ("square peg") ---
// Imagine this comes from a legacy library we cannot edit. It only speaks XML.
class LegacyXmlProvider {
public:
    // Different method name, different return shape than the Target.
    std::string requestXmlPayload() const {
        // A single blob of pseudo-XML records.
        return "<records>"
               "<user><name>Ada</name><role>admin</role></user>"
               "<user><name>Linus</name><role>dev</role></user>"
               "</records>";
    }
};

// --- Adapter: implements Target by wrapping and translating the Adaptee ---
// This is an *object* adapter: it holds the Adaptee by composition (not by
// inheritance), which keeps the coupling loose and swappable.
class XmlToJsonAdapter final : public JSONDataSource {
public:
    explicit XmlToJsonAdapter(std::shared_ptr<LegacyXmlProvider> provider)
        : provider_(std::move(provider)) {}

    std::vector<std::string> fetchAsJson() const override {
        // The core of the pattern: call the Adaptee, then convert its output
        // into the format the Target promises.
        const std::string xml = provider_->requestXmlPayload();
        return convertXmlToJson(xml);
    }

private:
    // Minimal, purpose-built parser: extracts <name>/<role> pairs and emits
    // JSON-like objects. Real code would use a proper parser; this stays small.
    static std::vector<std::string> convertXmlToJson(const std::string& xml) {
        std::vector<std::string> jsonRecords;
        std::size_t pos = 0;
        while ((pos = xml.find("<user>", pos)) != std::string::npos) {
            const std::string name = extractTag(xml, "name", pos);
            const std::string role = extractTag(xml, "role", pos);
            jsonRecords.push_back("{ \"name\": \"" + name +
                                  "\", \"role\": \"" + role + "\" }");
            pos += 6; // advance past this "<user>" to find the next record
        }
        return jsonRecords;
    }

    // Reads the text inside <tag>...</tag> found at/after 'from'.
    static std::string extractTag(const std::string& xml,
                                  const std::string& tag,
                                  std::size_t from) {
        const std::string open = "<" + tag + ">";
        const std::string close = "</" + tag + ">";
        const std::size_t start = xml.find(open, from);
        if (start == std::string::npos) return {};
        const std::size_t valueStart = start + open.size();
        const std::size_t end = xml.find(close, valueStart);
        if (end == std::string::npos) return {};
        return xml.substr(valueStart, end - valueStart);
    }

    std::shared_ptr<LegacyXmlProvider> provider_;
};

// --- Client: depends only on the Target interface, never on the Adaptee ---
void printReport(const JSONDataSource& source) {
    std::cout << "Client received JSON records:\n";
    for (const std::string& record : source.fetchAsJson()) {
        std::cout << "  " << record << "\n";
    }
}

int main() {
    // The legacy component we are forced to reuse.
    auto legacy = std::make_shared<LegacyXmlProvider>();

    // Wrap it so it satisfies the interface the client understands.
    XmlToJsonAdapter adapter(legacy);

    // The client is blissfully unaware that XML is involved underneath.
    printReport(adapter);

    return 0;
}
