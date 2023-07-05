import os
import re

# Create the HTML file
current_dir = os.path.dirname(os.path.abspath(__file__))
project_root = os.path.abspath(os.path.join(current_dir, '../../../../'))
benchmark_html = os.path.join(project_root, 'benchmark.html')

# GitLab URL
gitlab_url = 'https://gitlab.freedesktop.org/freetype/freetype/-/commit/'

# Directories
baseline_dir = os.path.join(project_root, 'src/tools/ftbench/baseline')
benchmark_dir = os.path.join(project_root, 'src/tools/ftbench/benchmark')

# Open HTML file for writing
with open(benchmark_html, 'w') as html_file:
    html_file.write('<html>\n<head>\n<title>Benchmark Results</title>\n</head>\n<body>\n<h1>Benchmark Results</h1>\n')

    # If it's the info file, we want to handle it differently
    with open(os.path.join(baseline_dir, "info.txt"), 'r') as f:
        baseline_info = f.readlines()
    with open(os.path.join(benchmark_dir, "info.txt"), 'r') as f:
        benchmark_info = f.readlines()

    # Check if commit ids are the same
    if baseline_info[1].strip() == benchmark_info[1].strip():
        html_file.write('<h2 style="color:red;">Warning: Baseline and Benchmark have the same commit ID</h2>\n')
        
    baseline_info[1] = '<a href="{}{}">{}</a>\n'.format(gitlab_url, baseline_info[1].strip(), baseline_info[1][:8])
    
    benchmark_info[1] = '<a href="{}{}">{}</a>\n'.format(gitlab_url, benchmark_info[1].strip(), benchmark_info[1][:8])

    # Write info to HTML
    html_file.write('<h2>Info</h2>\n')
    html_file.write('<table border="1">\n')
    html_file.write('<tr><th>Info</th><th>Baseline</th><th>Benchmark</th></tr>\n')
    info_list = ['Parameters', 'Commit ID', 'Commit Date', 'Branch']
    for info, baseline_line, benchmark_line in zip(info_list, baseline_info, benchmark_info):
        html_file.write('<tr><td>{}</td><td>{}</td><td>{}</td></tr>\n'.format(info, baseline_line.strip(), benchmark_line.strip()))
    html_file.write('</table><br/>\n')

    # Traverse through the 'baseline' directory
    for filename in os.listdir(baseline_dir):
        if filename != 'info.txt':
            
            with open(os.path.join(baseline_dir, filename), 'r') as f:
                baseline_results = f.readlines()
            with open(os.path.join(benchmark_dir, filename), 'r') as f:
                benchmark_results = f.readlines()
                
            # Get font name from within the .txt file
            for line in baseline_results:
                if line.startswith("ftbench results for font"):
                    fontname = line.split('/')[-1].strip("'")[:-2]


            # Write results to HTML
            html_file.write('<h2>Results for {}</h2>\n'.format(fontname))
            html_file.write('<table border="1">\n')
            html_file.write('<tr><th> Test </th><th> Baseline </th><th> Benchmark </th><th> Difference </th></tr>\n')

            for baseline_line, benchmark_line in zip(baseline_results, benchmark_results):
                if baseline_line.startswith('  '):
                    baseline_match = re.match(r'  (\w+(\s\(\w+\))?)\s+(\d+\.\d+)\s', baseline_line)
                    benchmark_match = re.match(r'  (\w+(\s\(\w+\))?)\s+(\d+\.\d+)\s', benchmark_line)

                    if baseline_match and benchmark_match:
                        baseline_value = float(baseline_match.group(3))
                        benchmark_value = float(benchmark_match.group(3))

                        # Calculate the percentage difference
                        percentage_diff =  ((baseline_value - benchmark_value) / baseline_value) * 100

                        if baseline_value > benchmark_value:
                            html_file.write('<tr><td>{}</td><td>{:.2f}\tus/op</td><td style="background-color: green;">{:.2f}\tus/op</td><td>{:.2f}%</td></tr>\n'.format(baseline_match.group(1), baseline_value, benchmark_value, percentage_diff))
                        else:
                            html_file.write('<tr><td>{}</td><td>{:.2f}\tus/op</td><td>{:.2f}\tus/op</td><td>{:.2f}%</td></tr>\n'.format(baseline_match.group(1), baseline_value, benchmark_value, percentage_diff))

            html_file.write('</table><br/>\n')

    html_file.write('<center>Freetype Benchmark</body>\n</html>\n')
