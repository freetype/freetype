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

        # Generate total results table
        generate_total_results_table(html_file, BASELINE_DIR, BENCHMARK_DIR)
        
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
    write_to_html(html_file, "<p>* Average time for all iterations. Smaller values are better.</p>")
    write_to_html(html_file, "<p>** N count in (x | y) format is for showing baseline and benchmark N counts seperately when they differs.</p>")
    

def generate_total_results_table(html_file, baseline_dir, benchmark_dir):
    """Prepare total results table for html"""
    
    # This dictionary will store aggregated results.
    test_results = {test: {"baseline": 0, "benchmark": 0, "n_baseline": 0, "n_benchmark": 0} for test in [
        "Load", "Load_Advances (Normal)", "Load_Advances (Fast)", "Load_Advances (Unscaled)", "Render",
        "Get_Glyph", "Get_Char_Index", "Iterate CMap", "New_Face", "Embolden", "Stroke", "Get_BBox",
        "Get_CBox", "New_Face & load glyph(s)"
    ]}
    
    for filename in os.listdir(baseline_dir):
        if filename.endswith(".txt") and not filename == "info.txt":
            
            baseline_results = read_file(os.path.join(baseline_dir, filename))
            benchmark_results = read_file(os.path.join(benchmark_dir, filename))
            
            for baseline_line, benchmark_line in zip(baseline_results, benchmark_results):
                if baseline_line.startswith("  "):
                    baseline_match = re.match(r"\s+(.*?)\s+(\d+\.\d+)\s+microseconds\s+(\d+)\s", baseline_line)
                    benchmark_match = re.match(r"\s+(.*?)\s+(\d+\.\d+)\s+microseconds\s+(\d+)\s", benchmark_line)
                    
                    if baseline_match and benchmark_match:
                        test = baseline_match.group(1).strip()
                        baseline_value = float(baseline_match.group(2))
                        benchmark_value = float(benchmark_match.group(2))
                        baseline_n = int(baseline_match.group(3))
                        benchmark_n = int(benchmark_match.group(3))
                        
                        # Aggregate the results
                        if test in test_results:
                            test_results[test]["baseline"] += baseline_value
                            test_results[test]["benchmark"] += benchmark_value
                            test_results[test]["n_baseline"] += baseline_n
                            test_results[test]["n_benchmark"] += benchmark_n
    
    # Writing to HTML
    write_to_html(html_file, "<h2>Total Results</h2>\n")
    write_to_html(html_file, '<table border="1">\n')
    write_to_html(
        html_file,
        '<tr><th>Test</th><th>N</th><th>Baseline (&#181;s)</th>\
        <th>Benchmark (&#181;s)</th><th>Difference (%)</th></tr>\n'
    )

    total_baseline = total_benchmark = total_diff = total_n_baseline = total_n_benchmark = 0
    
    for test, values in test_results.items():
        baseline = values["baseline"]
        benchmark = values["benchmark"]
        n_baseline = values["n_baseline"]
        n_benchmark = values["n_benchmark"]
        
        n_display = f"{n_baseline} | {n_benchmark}" if n_baseline != n_benchmark else str(n_baseline)
        
        diff = ((baseline - benchmark) / baseline) * 100

        # Calculate for total row
        total_baseline += baseline
        total_benchmark += benchmark
        total_n_baseline += n_baseline
        total_n_benchmark += n_benchmark
        
        # Check which value is smaller for color highlighting
        baseline_color = "highlight" if baseline <= benchmark else ""
        benchmark_color = "highlight" if benchmark <= baseline else ""

        write_to_html(
            html_file,
            f'<tr><td class="col1">{test}</td><td>{n_display}</td>\
            <td class="{baseline_color}">{baseline:.0f}</td>\
            <td class="{benchmark_color}">{benchmark:.0f}</td><td>{diff:.1f}</td></tr>\n'
        )

    total_diff = ((total_baseline - total_benchmark) / total_baseline) * 100
    total_n_display = f"{total_n_baseline} | {total_n_benchmark}" if total_n_baseline != total_n_benchmark else str(total_n_baseline)
    
    write_to_html(
        html_file,
        f'<tr><td class="col1">TOTAL</td><td class="col1">{total_n_display}</td>\
        <td class="col1">{total_baseline:.0f}</td><td class="col1">{total_benchmark:.0f}</td>\
        <td class="col1">{total_diff:.1f}</td></tr>\n'
    )
    
    write_to_html(html_file, "</table><br/>\n")



def generate_results_table(html_file, baseline_results, benchmark_results, filename):
    """Prepare results table for html"""
    fontname = [
        line.split("/")[-1].strip("'")[:-2]
        for line in baseline_results
        if line.startswith("ftbench results for font")
    ][0]

    write_to_html(html_file, "<h3>Results for {}</h2>\n".format(fontname))
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
