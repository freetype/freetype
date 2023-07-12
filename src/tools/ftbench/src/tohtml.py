"""This module converts the benchmark results into a HTML file."""
import os
import re
import sys

# Create the HTML file
PROJECT_ROOT = sys.argv[1]
BENCHMARK_HTML = os.path.join(PROJECT_ROOT, "benchmark.html")

# GitLab URL
GITLAB_URL = "https://gitlab.freedesktop.org/freetype/freetype/-/commit/"

# CSS style
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

# Directories
BASELINE_DIR = os.path.join(PROJECT_ROOT, "baseline")
BENCHMARK_DIR = os.path.join(PROJECT_ROOT, "benchmark")

# Open HTML file for writing and write the header
with open(BENCHMARK_HTML, "w") as html_file:
    html_file.write(
        f"<html>\n\
        <head>\n\
        <title>Benchmark Results</title>\n\
        {CSS_STYLE}\
        </head>\n\
        <body>\n\
        <h1>Benchmark Results</h1>\n"
    )

    # If it's the info file, we want to handle it differently
    with open(os.path.join(BASELINE_DIR, "info.txt"), "r") as f:
        baseline_info = f.readlines()
    with open(os.path.join(BENCHMARK_DIR, "info.txt"), "r") as f:
        benchmark_info = f.readlines()

    # Check if commit ids are the same
    if baseline_info[1].strip() == benchmark_info[1].strip():
        html_file.write(
            '<h2 class="warning">Warning: Baseline and Benchmark have the same commit ID</h2>\n'
        )

    baseline_info[1] = '<a href="{}{}">{}</a>\n'.format(
        GITLAB_URL, baseline_info[1].strip(), baseline_info[1][:8]
    )

    benchmark_info[1] = '<a href="{}{}">{}</a>\n'.format(
        GITLAB_URL, benchmark_info[1].strip(), benchmark_info[1][:8]
    )

    # Write info table
    html_file.write("<h2>Info</h2>\n")
    html_file.write('<table border="1">\n')
    html_file.write("<tr><th>Info</th><th>Baseline</th><th>Benchmark</th></tr>\n")
    info_list = ["Parameters", "Commit ID", "Commit Date", "Branch"]
    for info, baseline_line, benchmark_line in zip(
        info_list, baseline_info, benchmark_info
    ):
        html_file.write(
            '<tr><td class="col1">{}</td><td>{}</td><td>{}</td></tr>\n'.format(
                info, baseline_line.strip(), benchmark_line.strip()
            )
        )
    html_file.write("</table><br/>")
    html_file.write("*Smaller values mean faster operation<br/>\n")
    # Traverse through the 'baseline' directory
    for filename in os.listdir(BASELINE_DIR):
        if filename != "info.txt":
            with open(os.path.join(BASELINE_DIR, filename), "r") as f:
                baseline_results = f.readlines()
            with open(os.path.join(BENCHMARK_DIR, filename), "r") as f:
                benchmark_results = f.readlines()

            # Get font name from within the .txt file
            for line in baseline_results:
                if line.startswith("ftbench results for font"):
                    fontname = line.split("/")[-1]\
                        .strip("'")[:-2]

            # Write test column headers
            html_file.write("<h2>Results for {}</h2>\n".format(fontname))
            html_file.write('<table border="1">\n')
            html_file.write(
                '<tr>\
                <th>Test</th>\
                <th>N</th>\
                <th><a href="{}.txt">Baseline</a> (ms)</th>\
                <th><a href="{}.txt">Benchmark</a> (ms)</th>\
                <th>Difference (%)</th>\
                </tr>\n'.format(
                    os.path.join(BASELINE_DIR, fontname[:-4]),
                    os.path.join(BENCHMARK_DIR, fontname[:-4]),
                )
            )

            # Write test results
            for baseline_line, benchmark_line in zip(
                baseline_results, benchmark_results
            ):
                # Check if line is a test result
                # Get results by Regex
                if baseline_line.startswith("  "):
                    baseline_match = re.match(
                        r"\s+(.*?)\s+(\d+\.\d+)\s+ms\s+(\d+)\s", baseline_line
                    )

                    benchmark_match = re.match(
                        r"\s+(.*?)\s+(\d+\.\d+)\s+ms\s+(\d+)\s", benchmark_line
                    )

                    if baseline_match and benchmark_match:
                        baseline_value = float(baseline_match.group(2))
                        benchmark_value = float(benchmark_match.group(2))

                        # Calculate the percentage difference
                        percentage_diff = (
                            (baseline_value - benchmark_value) / baseline_value
                        ) * 100

                        # Get itaration number
                        baseline_n = baseline_match.group(3)
                        benchmark_n = benchmark_match.group(3)

                        # Check if iteration number is the same
                        if baseline_n == benchmark_n:
                            total_n = baseline_n
                        else:
                            total_n = baseline_n +\
                                " / " + benchmark_n

                        # Highlight the faster value
                        if baseline_value > benchmark_value:
                            html_file.write(
                                '<tr>\
                                <td class="col1">{}</td>\
                                <td>{}</td>\
                                <td class="lowlight">{:.3f}</td>\
                                <td class="highlight">{:.3f}</td>\
                                <td>{:.1f}</td>\
                                </tr>\n'.format(
                                    baseline_match.group(1),
                                    total_n,
                                    baseline_value,
                                    benchmark_value,
                                    percentage_diff,
                                )
                            )
                        else:
                            html_file.write(
                                '<tr>\
                                <td class="col1">{}</td>\
                                <td>{}</td>\
                                <td class="highlight">{:.3f}</td>\
                                <td class="lowlight">{:.3f}</td>\
                                <td>{:.1f}</td>\
                                </tr>\n'.format(
                                    baseline_match.group(1),
                                    total_n,
                                    baseline_value,
                                    benchmark_value,
                                    percentage_diff,
                                )
                            )

            html_file.write("</table><br/>\n")

    html_file.write("<center>Freetype Benchmark</center></body>\n</html>\n")
