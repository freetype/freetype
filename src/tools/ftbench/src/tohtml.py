"""This script generates a HTML file from the results of ftbench"""
import os
import re
import sys

GITLAB_URL = "https://gitlab.freedesktop.org/freetype/freetype/-/commit/"
CSS_STYLE = """
  <style>
    table {
        table-layout: fixed;
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
OBJ_DIR = sys.argv[1]
BASELINE_DIR = os.path.join(OBJ_DIR, "baseline")
BENCHMARK_DIR = os.path.join(OBJ_DIR, "benchmark")
BENCHMARK_HTML = os.path.join(OBJ_DIR, "benchmark.html")

FONT_COUNT = 5

WARNING_SAME_COMMIT = "Warning: Baseline and Benchmark have the same commit ID!"
INFO_1 = "* Average time for single iteration. Smaller values are better."
INFO_2 = "* If a value in the 'Iterations' column is given as '<i>x | y</i>', values <i>x</i> and <i>y</i> give the number of iterations in the baseline and the benchmark test, respectively."


def main():
    """Entry point for theq script"""
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
                f'<h2 class="warning">{WARNING_SAME_COMMIT}</h2>\n',
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
    info[1] = f'<a href="{GITLAB_URL}{info[1].strip()}">{info[1][:8]}</a>\n'
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
            f'<tr><td class="col1">{info}</td><td>{baseline_line.strip()}</td><td>{benchmark_line.strip()}</td></tr>\n'
        )
    write_to_html(html_file, "</table><br/>")
    write_to_html(html_file, f"<p>{INFO_1}</p>")
    write_to_html(html_file, f"<p>{INFO_2}</p>")


def generate_total_results_table(html_file, baseline_dir, benchmark_dir):
    """Prepare total results table for html"""

    # This dictionary will store aggregated results.
    test_results = {
        test: {"baseline": 0, "benchmark": 0, "n_baseline": 0, "n_benchmark": 0}
        for test in [
            "Load",
            "Load_Advances (Normal)",
            "Load_Advances (Fast)",
            "Load_Advances (Unscaled)",
            "Render",
            "Get_Glyph",
            "Get_Char_Index",
            "Iterate CMap",
            "New_Face",
            "Embolden",
            "Stroke",
            "Get_BBox",
            "Get_CBox",
            "New_Face & load glyph(s)",
        ]
    }

    total_time = 0

    for filename in os.listdir(baseline_dir):
        if filename.endswith(".txt") and not filename == "info.txt":
            baseline_results = read_file(os.path.join(baseline_dir, filename))
            benchmark_results = read_file(os.path.join(benchmark_dir, filename))

            for baseline_line, benchmark_line in zip(
                baseline_results, benchmark_results
            ):
                if baseline_line.startswith("Total time:"):
                    baseline_match = re.match(r"Total time: (\d+)s", baseline_line)
                    benchmark_match = re.match(r"Total time: (\d+)s", benchmark_line)

                    if baseline_match and benchmark_match:
                        total_time += int(baseline_match.group(1))
                        total_time += int(benchmark_match.group(1))

                if baseline_line.startswith("  "):
                    baseline_match = re.match(
                        r"\s+(.*?)\s+(\d+\.\d+)\s+microseconds\s+(\d+)\s", baseline_line
                    )
                    benchmark_match = re.match(
                        r"\s+(.*?)\s+(\d+\.\d+)\s+microseconds\s+(\d+)\s",
                        benchmark_line,
                    )

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
        "<tr><th>Test</th><th>Iterations</th><th>* Baseline (&#181;s)</th>\
        <th>* Benchmark (&#181;s)</th><th>Difference (%)</th></tr>\n",
    )

    total_baseline = total_benchmark = total_n_baseline = total_n_benchmark = 0

    for test, values in test_results.items():
        baseline = values["baseline"] / FONT_COUNT
        benchmark = values["benchmark"] / FONT_COUNT
        n_baseline = values["n_baseline"] / FONT_COUNT
        n_benchmark = values["n_benchmark"] / FONT_COUNT

        n_display = (
            f"{n_baseline:.0f} | {n_benchmark:.0f}"
            if n_baseline != n_benchmark
            else int(n_baseline)
        )

        diff = (
            ((baseline - benchmark) / baseline) * 100
            if not (baseline - benchmark) == 0
            else 0
        )

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
            <td class="{baseline_color}">{baseline:.1f}</td>\
            <td class="{benchmark_color}">{benchmark:.1f}</td><td>{diff:.1f}</td></tr>\n',
        )

    write_to_html(
        html_file,
        f'<tr><td class="col1">Total duration for all tests:</td><td class="col1" colspan="4">{total_time:.0f} s</td>',
    )

    write_to_html(html_file, "</table>\n")


def generate_results_table(html_file, baseline_results, benchmark_results, filename):
    """Prepare results table for html"""
    fontname = [
        line.split("/")[-1].strip("'")[:-2]
        for line in baseline_results
        if line.startswith("ftbench results for font")
    ][0]

    write_to_html(html_file, f"<h3>Results for {fontname}</h2>\n")
    write_to_html(html_file, '<table border="1">\n')
    write_to_html(
        html_file,
        f'<tr><th>Test</th><th>Iterations</th>\
        <th>* <a href="{ os.path.join("./baseline/", filename[:-4])}.txt">Baseline</a> (&#181;s)</th>\
        <th>* <a href="{ os.path.join("./benchmark/", filename[:-4])}.txt">Benchmark</a> (&#181;s)</th>\
        <th>Difference (%)</th></tr>\n'
    )

    total_n = total_time = 0

    for baseline_line, benchmark_line in zip(baseline_results, benchmark_results):
        if baseline_line.startswith("Total time:"):
            baseline_match = re.match(r"Total time: (\d+)s", baseline_line)
            benchmark_match = re.match(r"Total time: (\d+)s", benchmark_line)

            if baseline_match and benchmark_match:
                total_time += int(baseline_match.group(1))
                total_time += int(benchmark_match.group(1))

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
                    ((baseline_value - benchmark_value) / baseline_value) * 100
                    if not (baseline_value - benchmark_value) == 0
                    else 0
                )

                baseline_n = baseline_match.group(3)
                benchmark_n = benchmark_match.group(3)

                n = (
                    baseline_n
                    if baseline_n == benchmark_n
                    else baseline_n + " | " + benchmark_n
                )

                total_n += int(baseline_n)
                total_n += int(benchmark_n)

                # Check which value is smaller for color highlighting
                baseline_color = (
                    "highlight" if baseline_value <= benchmark_value else ""
                )
                benchmark_color = (
                    "highlight" if benchmark_value <= baseline_value else ""
                )

                write_to_html(
                    html_file,
                    f'<tr><td class="col1">{baseline_match.group(1)}</td><td>{n}</td>\
                    <td class="{baseline_color}">{baseline_value:.1f}</td><td class="{benchmark_color}">{benchmark_value:.1f}</td><td>{percentage_diff:.1f}</td></tr>\n',
                )

    write_to_html(
        html_file,
        f'<tr><td class="col1">Total duration for the font:</td><td class="col1" colspan="4">{total_time:.0f} s</td></table>\n',
    )


if __name__ == "__main__":
    main()
