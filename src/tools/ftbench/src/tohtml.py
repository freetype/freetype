import os
import re

# Create the HTML file
with open('benchmark_results.html', 'w') as f:
    f.write('<html>\n')
    f.write('<head>\n')
    f.write('<title>Benchmark Results</title>\n')
    f.write('</head>\n')
    f.write('<body>\n')
    f.write('<h1>Benchmark Results</h1>\n')

    # Traverse through the 'baselines' directory
    for filename in os.listdir('baselines'):
        baseline_filepath = os.path.join('baselines', filename)
        benchmark_filepath = os.path.join('benchmarks', filename)

        # Process the baseline file
        with open(baseline_filepath, 'r') as baseline_file:
            baseline_lines = baseline_file.readlines()

        # Process the benchmark file
        with open(benchmark_filepath, 'r') as benchmark_file:
            benchmark_lines = benchmark_file.readlines()

        f.write(f'<h2>Results for {filename}</h2>\n')
        f.write('<table border="1">\n')
        f.write('<tr><th>Test</th><th>Baseline</th><th>Current</th></tr>\n')

        # For each line in the baseline and benchmark files
        for baseline_line, benchmark_line in zip(baseline_lines, benchmark_lines):
            # If the line starts with a space, it's a test result line
            if baseline_line.startswith('  '):
                # Extract the test name, the time per operation, and the number of operations done
                baseline_match = re.match(r'  (\w+(\s\(\w+\))?)\s+(\d+\.\d+)\s', baseline_line)
                benchmark_match = re.match(r'  (\w+(\s\(\w+\))?)\s+(\d+\.\d+)\s', benchmark_line)

                # If the line could be parsed
                if baseline_match and benchmark_match:
                    # Check which value is higher
                    baseline_value = float(baseline_match.group(3))
                    benchmark_value = float(benchmark_match.group(3))

                    # Write the test result to the HTML file
                    if baseline_value > benchmark_value:
                        f.write(f'<tr><td>{baseline_match.group(1)}</td><td style="background-color: green;">{baseline_match.group(3)}</td><td>{benchmark_match.group(3)}</td></tr>\n')
                    else:
                        f.write(f'<tr><td>{baseline_match.group(1)}</td><td>{baseline_match.group(3)}</td><td style="background-color: green;">{benchmark_match.group(3)}</td></tr>\n')

        f.write('</table>\n')

    f.write('</body>\n')
    f.write('</html>\n')
