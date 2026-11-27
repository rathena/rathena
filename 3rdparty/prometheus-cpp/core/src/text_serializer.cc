#include "prometheus/text_serializer.h"

#include <cmath>
#include <limits>
#include <locale>
#include <ostream>
#include <string>

#include "prometheus/client_metric.h"
#include "prometheus/metric_family.h"
#include "prometheus/metric_type.h"

namespace prometheus {

namespace {

// Write a double as a string, with proper formatting for infinity and NaN
void WriteValue(std::ostream& out, double value) {
  if (std::isnan(value)) {
    out << "Nan";
  } else if (std::isinf(value)) {
    out << (value < 0 ? "-Inf" : "+Inf");
  } else {
    out << value;
  }
}

void WriteValue(std::ostream& out, const std::string& value) {
  for (auto c : value) {
    switch (c) {
      case '\n':
        out << '\\' << 'n';
        break;

      case '\\':
        out << '\\' << c;
        break;

      case '"':
        out << '\\' << c;
        break;

      default:
        out << c;
        break;
    }
  }
}

// Write a line header: metric name and labels
template <typename T = std::string>
void WriteHead(std::ostream& out, const MetricFamily& family,
               const ClientMetric& metric, const std::string& suffix = "",
               const std::string& extraLabelName = "",
               const T& extraLabelValue = T()) {
  out << family.name << suffix;
  if (!metric.label.empty() || !extraLabelName.empty()) {
    out << "{";
    const char* prefix = "";

    for (auto& lp : metric.label) {
      out << prefix << lp.name << "=\"";
      WriteValue(out, lp.value);
      out << "\"";
      prefix = ",";
    }
    if (!extraLabelName.empty()) {
      out << prefix << extraLabelName << "=\"";
      WriteValue(out, extraLabelValue);
      out << "\"";
    }
    out << "}";
  }
  out << " ";
}

// Write a line trailer: timestamp
void WriteTail(std::ostream& out, const ClientMetric& metric) {
  if (metric.timestamp_ms != 0) {
    out << " " << metric.timestamp_ms;
  }
  out << "\n";
}

void SerializeCounter(std::ostream& out, const MetricFamily& family,
                      const ClientMetric& metric) {
  WriteHead(out, family, metric);
  WriteValue(out, metric.counter.value);
  WriteTail(out, metric);
}

void SerializeGauge(std::ostream& out, const MetricFamily& family,
                    const ClientMetric& metric) {
  WriteHead(out, family, metric);
  WriteValue(out, metric.gauge.value);
  WriteTail(out, metric);
}

void SerializeInfo(std::ostream& out, const MetricFamily& family,
                   const ClientMetric& metric) {
  WriteHead(out, family, metric, "_info");
  WriteValue(out, metric.info.value);
  WriteTail(out, metric);
}

void SerializeSummary(std::ostream& out, const MetricFamily& family,
                      const ClientMetric& metric) {
  auto& sum = metric.summary;
  WriteHead(out, family, metric, "_count");
  out << sum.sample_count;
  WriteTail(out, metric);

  WriteHead(out, family, metric, "_sum");
  WriteValue(out, sum.sample_sum);
  WriteTail(out, metric);

  for (auto& q : sum.quantile) {
    WriteHead(out, family, metric, "", "quantile", q.quantile);
    WriteValue(out, q.value);
    WriteTail(out, metric);
  }
}

void SerializeUntyped(std::ostream& out, const MetricFamily& family,
                      const ClientMetric& metric) {
  WriteHead(out, family, metric);
  WriteValue(out, metric.untyped.value);
  WriteTail(out, metric);
}

void SerializeHistogram(std::ostream& out, const MetricFamily& family,
                        const ClientMetric& metric) {
  auto& hist = metric.histogram;
  WriteHead(out, family, metric, "_count");
  out << hist.sample_count;
  WriteTail(out, metric);

  WriteHead(out, family, metric, "_sum");
  WriteValue(out, hist.sample_sum);
  WriteTail(out, metric);

  double last = -std::numeric_limits<double>::infinity();
  for (auto& b : hist.bucket) {
    WriteHead(out, family, metric, "_bucket", "le", b.upper_bound);
    last = b.upper_bound;
    out << b.cumulative_count;
    WriteTail(out, metric);
  }

  if (last != std::numeric_limits<double>::infinity()) {
    WriteHead(out, family, metric, "_bucket", "le", "+Inf");
    out << hist.sample_count;
    WriteTail(out, metric);
  }
}

void SerializeFamily(std::ostream& out, const MetricFamily& family) {
  if (!family.help.empty()) {
    out << "# HELP " << family.name << " " << family.help << "\n";
  }
  switch (family.type) {
    case MetricType::Counter:
      out << "# TYPE " << family.name << " counter\n";
      for (auto& metric : family.metric) {
        SerializeCounter(out, family, metric);
      }
      break;
    case MetricType::Gauge:
      out << "# TYPE " << family.name << " gauge\n";
      for (auto& metric : family.metric) {
        SerializeGauge(out, family, metric);
      }
      break;
    // info is not handled by prometheus, we use gauge as workaround
    // (https://github.com/OpenObservability/OpenMetrics/blob/98ae26c87b1c3bcf937909a880b32c8be643cc9b/specification/OpenMetrics.md#info-1)
    case MetricType::Info:
      out << "# TYPE " << family.name << " gauge\n";
      for (auto& metric : family.metric) {
        SerializeInfo(out, family, metric);
      }
      break;
    case MetricType::Summary:
      out << "# TYPE " << family.name << " summary\n";
      for (auto& metric : family.metric) {
        SerializeSummary(out, family, metric);
      }
      break;
    case MetricType::Untyped:
      out << "# TYPE " << family.name << " untyped\n";
      for (auto& metric : family.metric) {
        SerializeUntyped(out, family, metric);
      }
      break;
    case MetricType::Histogram:
      out << "# TYPE " << family.name << " histogram\n";
      for (auto& metric : family.metric) {
        SerializeHistogram(out, family, metric);
      }
      break;
  }
}
}  // namespace

void TextSerializer::Serialize(std::ostream& out,
                               const std::vector<MetricFamily>& metrics) const {
  auto saved_locale = out.getloc();
  auto saved_precision = out.precision();

  out.imbue(std::locale::classic());
  out.precision(std::numeric_limits<double>::max_digits10 - 1);

  for (auto& family : metrics) {
    SerializeFamily(out, family);
  }

  out.imbue(saved_locale);
  out.precision(saved_precision);
}
}  // namespace prometheus
