"""This script generates a HTML file from the results of ftbench"""
# Ahmet Goksu ahmet@goksu.in ahmet.goksu.in

import os
import re
import sys

PROJECT_ROOT = sys.argv[1]
BENCHMARK_HTML = os.path.join(PROJECT_ROOT, "benchmark.html")
GITLAB_URL = "https://gitlab.freedesktop.org/freetype/freetype/-/commit/"
CSS_STYLE = """
  <style>
    table {
        
        }
    th, td {
        padding: 3px;
      text-align: center;
    }
    th {
      background-color: #ccc;
      color: black;
    }
    .warning{
        color: red;
    }
    .col1 {
        background-color: #eee;
        }   
    
    .highlight {
      background-color: #0a0;
    }
  </style>
"""
BASELINE_DIR = os.path.join(PROJECT_ROOT, "baseline")
BENCHMARK_DIR = os.path.join(PROJECT_ROOT, "benchmark")

def main():
    """Entry point for the script"""    
    with open(BENCHMARK_HTML, "w") as html_file:
        write_to_html(html_file, "<html>\n<head>\n")
        write_to_html(html_file, CSS_STYLE)
        write_to_html(html_file, "</head>\n<body>\n")
        write_to_html(html_file, "<h1>Freetype Benchmark Results</h1>\n")

        baseline_info = parse_info_file(os.path.join(BASELINE_DIR, "info.txt"))
        benchmark_info = parse_info_file(os.path.join(BENCHMARK_DIR, "info.txt"))

        if baseline_info[1].strip() == benchmark_info[1].strip():
            write_to_html(
                html_file,
                '<h2 class="warning">Warning: Baseline and Benchmark have the same commit ID!</h2>\n',
            )

        generate_info_table(html_file, baseline_info, benchmark_info)

        # Generate results tables
        for filename in os.listdir(BASELINE_DIR):
            if filename.endswith(".txt") and not filename == "info.txt":
                baseline_results = read_file(os.path.join(BASELINE_DIR, filename))
                benchmark_results = read_file(os.path.join(BENCHMARK_DIR, filename))

                generate_results_table(
                    html_file, baseline_results, benchmark_results, filename
                )
        write_to_html(html_file, "<center>Freetype Benchmark</center>\n")
        write_to_html(html_file, "</body>\n</html>\n")

def write_to_html(html_file, content):
    """Write content to html file"""
    html_file.write(content)


def read_file(file_path):
    """Read file and return list of lines"""
    with open(file_path, "r") as f:
        return f.readlines()


def parse_info_file(info_file):
    """Get info from info.txt file and return as list"""
    info = read_file(info_file)
    info[1] = '<a href="{}{}">{}</a>\n'.format(GITLAB_URL, info[1].strip(), info[1][:8])
    return info


def generate_info_table(html_file, baseline_info, benchmark_info):
    """Prepare info table for html"""
    write_to_html(html_file, "<h2>Info</h2>\n")
    write_to_html(html_file, '<table border="1">\n')
    write_to_html(
        html_file, "<tr><th>Info</th><th>Baseline</th><th>Benchmark</th></tr>\n"
    )
    info_list = ["Parameters", "Commit ID", "Commit Date", "Branch"]
    for info, baseline_line, benchmark_line in zip(
        info_list, baseline_info, benchmark_info
    ):
        write_to_html(
            html_file,
            '<tr><td class="col1">{}</td><td>{}</td><td>{}</td></tr>\n'.format(
                info, baseline_line.strip(), benchmark_line.strip()
            ),
        )
    write_to_html(html_file, "</table><br/>")
    write_to_html(html_file, "* Cumulative time for iterations which is better in smaller values<br/>\n")


def generate_results_table(html_file, baseline_results, benchmark_results, filename):
    """Prepare results table for html"""
    fontname = [
        line.split("/")[-1].strip("'")[:-2]
        for line in baseline_results
        if line.startswith("ftbench results for font")
    ][0]

    write_to_html(html_file, "<h2>Results for {}</h2>\n".format(fontname))
    write_to_html(html_file, '<table border="1">\n')
    write_to_html(
        html_file,
        '<tr><th>Test</th><th>N</th>\
        <th>* <a href="{}.txt">Baseline</a> (&#181;s)</th>\
        <th>* <a href="{}.txt">Benchmark</a> (&#181;s)</th>\
        <th>Difference (%)</th></tr>\n'.format(
            os.path.join(BASELINE_DIR, filename[:-4]),
            os.path.join(BENCHMARK_DIR, filename[:-4]),
        ),
    )

    total_n = total_time_baseline = total_time_benchmark = total_difference = 0

    for baseline_line, benchmark_line in zip(baseline_results, benchmark_results):
        if baseline_line.startswith("  "):
            baseline_match = re.match(
                r"\s+(.*?)\s+(\d+\.\d+)\s+microseconds\s+(\d+)\s", baseline_line
            )
            benchmark_match = re.match(
                r"\s+(.*?)\s+(\d+\.\d+)\s+microseconds\s+(\d+)\s", benchmark_line
            )

            if baseline_match and benchmark_match:
                baseline_value = float(baseline_match.group(2))
                benchmark_value = float(benchmark_match.group(2))

                percentage_diff = (
                    (baseline_value - benchmark_value) / baseline_value
                ) * 100

                baseline_n = baseline_match.group(3)
                benchmark_n = benchmark_match.group(3)

                n = (
                    baseline_n
                    if baseline_n == benchmark_n
                    else baseline_n + " | " + benchmark_n
                )

                total_n += int(baseline_n)
                total_n += int(benchmark_n)
                total_time_baseline += baseline_value
                total_time_benchmark += benchmark_value
                

                if baseline_value > benchmark_value:
                    write_to_html(
                        html_file,
                        '<tr><td class="col1">{}</td><td>{}</td>\
                        <td class="lowlight">{:.0f}</td><td class="highlight">{:.0f}</td><td>{:.1f}</td></tr>\n'.format(
                            baseline_match.group(1),
                            n,
                            baseline_value,
                            benchmark_value,
                            percentage_diff,
                        ),
                    )
                else:
                    write_to_html(
                        html_file,
                        '<tr><td class="col1">{}</td><td>{}</td>\
                        <td class="highlight">{:.0f}</td><td class="lowlight">{:.0f}</td><td>{:.1f}</td></tr>\n'.format(
                            baseline_match.group(1),
                            n,
                            baseline_value,
                            benchmark_value,
                            percentage_diff,
                        ),
                    )

    write_to_html(
        html_file,
        '<tr><td class="col1">TOTAL</td><td class="col1">{}</td>\
                        <td class="col1">{:.0f}</td><td class="col1">{:.0f}</td><td class="col1">{:.1f}</td></tr>\n'.format(
            total_n, total_time_baseline, total_time_benchmark, (total_time_baseline - total_time_benchmark) / total_time_baseline * -100
        ),
    )
    write_to_html(html_file, "</table><br/>\n")

if __name__ == "__main__":
    main()
