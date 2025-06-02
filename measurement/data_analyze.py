#!/usr/bin/env python3

import os
import sys
import math
import statistics
from pathlib import Path

def read_data_file(filename, skip_data_rows=10):
    """
    Read the data file and parse it into columns.
    Returns header and data as lists.
    
    Args:
        filename (str): Path to the data file
        skip_data_rows (int): Number of data rows to skip after header (default: 10)
    """
    try:
        with open(filename, 'r') as f:
            lines = f.readlines()
    except FileNotFoundError:
        return None, None
    
    # Remove whitespace and filter empty lines
    lines = [line.strip() for line in lines if line.strip()]
    
    if len(lines) < 2:
        return None, None
    
    # Define the correct column names
    header = ["Clock", "Core cyc", "Instruct", "Uops", "res.stl.", "L1D Miss", "CodeMiss"]
    
    # Skip the header line and the specified number of data rows
    data_lines = lines[1 + skip_data_rows:]
    
    # Parse data rows - each row should have exactly 7 values
    data_rows = []
    for line in data_lines:
        values = line.split()
        if len(values) != 7:
            continue
        try:
            row = [float(x) for x in values]
            data_rows.append(row)
        except ValueError:
            continue
    
    if not data_rows:
        return None, None
    
    # Convert to column-wise data
    num_cols = 7
    columns = [[] for _ in range(num_cols)]
    
    for row in data_rows:
        for i, value in enumerate(row):
            columns[i].append(value)
    
    return header, columns

def calculate_filtered_mean(data, sigma_threshold=2.0):
    """
    DEPRECATED: This function is no longer used.
    See select_median_repetition() for the new approach.
    """
    if len(data) < 2:
        return statistics.mean(data) if data else None
    
    # Calculate initial mean and standard deviation
    mean_val = statistics.mean(data)
    std_dev = statistics.stdev(data)
    
    if std_dev == 0:
        return mean_val
    
    # Filter data within sigma_threshold standard deviations
    filtered_data = []
    threshold = sigma_threshold * std_dev
    
    for value in data:
        if abs(value - mean_val) <= threshold:
            filtered_data.append(value)
    
    # Return filtered mean, or original mean if no data passes filter
    if filtered_data:
        return statistics.mean(filtered_data)
    else:
        return mean_val

def select_median_repetition(run_data):
    """
    Select the single repetition whose Core cyc lies in the middle (median).
    If multiple repetitions have the same median Core cyc, choose the first one.
    
    Args:
        run_data (dict): Dictionary with column names as keys and lists of repetition values
    
    Returns:
        dict: Dictionary with column names as keys and single selected values, or None if failed
    """
    if "Core cyc" not in run_data or not run_data["Core cyc"]:
        return None
    
    core_cyc_values = run_data["Core cyc"]
    
    if len(core_cyc_values) == 0:
        return None
    
    # Find the median Core cyc value
    sorted_core_cyc = sorted(core_cyc_values)
    n = len(sorted_core_cyc)
    
    if n % 2 == 0:
        # Even number of values - use the lower of the two middle values
        median_core_cyc = sorted_core_cyc[n//2 - 1]
    else:
        # Odd number of values - use the middle value
        median_core_cyc = sorted_core_cyc[n//2]
    
    # Find the FIRST repetition that has this median Core cyc value
    selected_index = None
    for i, core_cyc_val in enumerate(core_cyc_values):
        if core_cyc_val == median_core_cyc:
            selected_index = i
            break  # Take the first occurrence
    
    # If no exact match (shouldn't happen), find the closest
    if selected_index is None:
        closest_diff = float('inf')
        selected_index = 0
        for i, core_cyc_val in enumerate(core_cyc_values):
            diff = abs(core_cyc_val - median_core_cyc)
            if diff < closest_diff:
                closest_diff = diff
                selected_index = i
    
    # Extract the values from the selected repetition for all columns
    selected_values = {}
    for col_name, col_data in run_data.items():
        if selected_index < len(col_data):
            selected_values[col_name] = col_data[selected_index]
        else:
            selected_values[col_name] = None
    
    return selected_values

def analyze_single_run(filename):
    """
    Analyze a single run file and return the selected median repetition values for each column.
    
    Args:
        filename (str): Path to the run file
    
    Returns:
        dict: Dictionary with column names as keys and selected median values, or None if failed
    """
    header, columns = read_data_file(filename)
    
    if header is None or columns is None:
        return None
    
    # Organize data by repetition (transpose columns to rows)
    if not columns or not columns[0]:
        return None
    
    num_repetitions = len(columns[0])
    run_data = {}
    
    # Build run_data dictionary where each column contains all repetitions
    for i, col_name in enumerate(header):
        if i < len(columns):
            run_data[col_name] = columns[i]
        else:
            run_data[col_name] = [None] * num_repetitions
    
    # Select the median repetition based on clock cycle
    selected_values = select_median_repetition(run_data)
    
    return selected_values

def apply_second_level_filtering(all_means, sigma_threshold=2.0):
    """
    Apply second-level filtering based on Core cyc column only, but apply to all columns.
    This removes data lines where Core cyc is outside mean ± 2σ of the collected dataset.
    
    Args:
        all_means (dict): Dictionary with column names as keys and lists of selected values
        sigma_threshold (float): Threshold in standard deviations for outlier detection
    
    Returns:
        tuple: (filtered_means_dict, filtering_summary_dict)
    """
    filtered_means = {}
    filtering_summary = {}
    
    header = ["Clock", "Core cyc", "Instruct", "Uops", "res.stl.", "L1D Miss", "CodeMiss"]
    
    # First, determine which runs to keep based on Core cyc column only
    if "Core cyc" not in all_means or not all_means["Core cyc"]:
        # No Core cyc data, return empty results
        for col_name in header:
            filtered_means[col_name] = []
            filtering_summary[col_name] = {"original": 0, "filtered": 0, "removed": 0}
        return filtered_means, filtering_summary
    
    # Get valid Core cyc values and their indices
    core_cyc_values = all_means["Core cyc"]
    valid_indices = []
    valid_core_cyc_values = []
    
    for i, core_cyc_val in enumerate(core_cyc_values):
        if core_cyc_val is not None:
            valid_indices.append(i)
            valid_core_cyc_values.append(core_cyc_val)
    
    if len(valid_core_cyc_values) < 2:
        # Not enough data for filtering, keep all valid data
        keep_indices = valid_indices
    else:
        # Apply second-level filtering based on Core cyc values only
        core_cyc_median = statistics.median(valid_core_cyc_values)
        core_cyc_std = statistics.stdev(valid_core_cyc_values)
        
        if core_cyc_std == 0:
            # No variation in Core cyc, keep all data
            keep_indices = valid_indices
        else:
            # Filter based on Core cyc values within sigma_threshold standard deviations
            threshold = sigma_threshold * core_cyc_std
            keep_indices = []
            
            for i, core_cyc_val in zip(valid_indices, valid_core_cyc_values):
                if abs(core_cyc_val - core_cyc_median) <= threshold:
                    keep_indices.append(i)
    
    # Now apply the same filtering (based on Core cyc) to all columns
    for col_name in header:
        if col_name not in all_means or not all_means[col_name]:
            filtered_means[col_name] = []
            filtering_summary[col_name] = {"original": 0, "filtered": 0, "removed": 0}
            continue
        
        col_values = all_means[col_name]
        
        # Count original valid values
        original_count = sum(1 for val in col_values if val is not None)
        
        # Filter using the indices determined by Core cyc filtering
        filtered_values = []
        for i in keep_indices:
            if i < len(col_values) and col_values[i] is not None:
                filtered_values.append(col_values[i])
        
        filtered_means[col_name] = filtered_values
        filtering_summary[col_name] = {
            "original": original_count,
            "filtered": len(filtered_values),
            "removed": original_count - len(filtered_values)
        }
    
    return filtered_means, filtering_summary

def save_filtered_means(all_means, output_file, double_filtered_means=None):
    """
    Save all selected median values to a single comprehensive file with proper alignment.
    
    Args:
        all_means (dict): Dictionary with column names as keys and lists of selected values
        output_file (str): Path to output file
        double_filtered_means (dict): Dictionary with double-filtered values (optional)
    """
    header = ["Clock", "Core cyc", "Instruct", "Uops", "res.stl.", "L1D Miss", "CodeMiss"]
    
    # Define column widths for proper alignment
    col_widths = {
        "Run": 8,
        "Clock": 10, 
        "Core cyc": 10,
        "Instruct": 10,
        "Uops": 10,
        "res.stl.": 10,
        "L1D Miss": 10,
        "CodeMiss": 10
    }
    
    def format_value(value):
        """Format a value as integer if it's a whole number, otherwise as float"""
        if value is None:
            return "N/A"
        # Check if the value is effectively an integer (difference from rounded value is very small)
        if abs(value - round(value)) < 1e-9:
            return str(int(round(value)))
        else:
            return f"{value:.4f}"
    
    def format_row(run_num, values, is_header=False):
        """Format a single row with proper alignment"""
        if is_header:
            formatted = f"{'Run':<{col_widths['Run']}}"
            for col_name in header:
                formatted += f"{col_name:<{col_widths[col_name]}}"
        else:
            formatted = f"{run_num:<{col_widths['Run']}}"
            for i, col_name in enumerate(header):
                if i < len(values) and values[i] is not None:
                    formatted_value = format_value(values[i])
                    formatted += f"{formatted_value:<{col_widths[col_name]}}"
                else:
                    formatted += f"{'N/A':<{col_widths[col_name]}}"
        return formatted.rstrip()  # Remove trailing spaces
    
    with open(output_file, 'w') as f:
        # Write comprehensive header with metadata
        f.write("# PMC Benchmark Results - Selected Median Core cyc Repetitions\n")
        f.write("# Method: Each run selects repetition with median Core cyc value\n")
        f.write("# Second-level filtering: Remove data lines with Core cyc outliers (2σ) from collection\n")
        f.write("#\n")
        f.write("# Data sections:\n")
        f.write("# 1. UNFILTERED: All selected median Core cyc repetitions\n")
        f.write("# 2. CORE_CYC_FILTERED: After removing Core cyc outliers from collection\n")
        f.write("#\n")
        f.write("\n")
        
        # Section 1: Unfiltered data
        f.write("=== UNFILTERED DATA (All Selected Median Core cyc Repetitions) ===\n")
        
        # Write header with proper alignment
        f.write(format_row(0, [], is_header=True) + "\n")
        f.write("-" * sum(col_widths.values()) + "\n")
        
        # Determine number of runs
        num_runs = 0
        for col_name in header:
            if col_name in all_means and all_means[col_name]:
                num_runs = max(num_runs, len(all_means[col_name]))
        
        # Write unfiltered data with proper alignment
        for run_idx in range(num_runs):
            values = []
            for col_name in header:
                if col_name in all_means and all_means[col_name] and run_idx < len(all_means[col_name]):
                    values.append(all_means[col_name][run_idx])
                else:
                    values.append(None)
            
            f.write(format_row(run_idx + 1, values) + "\n")
        
        # Section 2: Core cyc filtered data
        if double_filtered_means is not None:
            f.write("\n")
            f.write("=== CORE CYC FILTERED DATA (Outlier Data Lines Removed) ===\n")
            
            # Write header with proper alignment  
            f.write(format_row(0, [], is_header=True) + "\n")
            f.write("-" * sum(col_widths.values()) + "\n")
            
            # Determine maximum number of filtered values
            max_filtered = 0
            for col_name in header:
                if col_name in double_filtered_means and double_filtered_means[col_name]:
                    max_filtered = max(max_filtered, len(double_filtered_means[col_name]))
            
            # Write filtered data with proper alignment
            for idx in range(max_filtered):
                values = []
                for col_name in header:
                    if (col_name in double_filtered_means and 
                        double_filtered_means[col_name] and 
                        idx < len(double_filtered_means[col_name])):
                        values.append(double_filtered_means[col_name][idx])
                    else:
                        values.append(None)
                
                f.write(format_row(idx + 1, values) + "\n")

def create_summary_table_with_double_filtering(all_means, num_runs, second_level_sigma):
    """
    Create a summary table with double filtering information and proper alignment.
    
    Args:
        all_means (dict): Dictionary with column names as keys and lists of means as values
        num_runs (int): Total number of runs attempted
        second_level_sigma (float): Sigma threshold for second-level filtering
    """
    print(f"=== MULTI-RUN SUMMARY TABLE ({num_runs} runs) ===")
    print()
    
    # Check how many runs were successful
    successful_runs = 0
    for col_name, means_list in all_means.items():
        if means_list:
            successful_runs = len([m for m in means_list if m is not None])
            break
    
    print(f"Total runs attempted: {num_runs}")
    print(f"Successful runs: {successful_runs}")
    print(f"Each run: 100 repetitions → select repetition with median Core cyc")
    print()
    
    if successful_runs == 0:
        print("No successful runs to analyze!")
        return
    
    # Apply second-level filtering to remove outlier data lines
    # print(f"Applying second-level filtering (removing data lines with Core cyc beyond {second_level_sigma}σ)...")
    # print("Note: Filtering criteria based on Core cyc values from the collected dataset")
    filtered_means, filtering_summary = apply_second_level_filtering(all_means, sigma_threshold=second_level_sigma)
    
    # # Show filtering summary with proper alignment
    # print()
    # print("Second-level filtering summary (Core cyc-based filtering applied to all):")
    # header_format = f"{'Column':<12} {'Original':>10} {'Filtered':>10} {'Removed':>10} {'Keep %':>10}"
    # print(header_format)
    # print("-" * len(header_format))
    
    # for col_name in ["Clock", "Core cyc", "Instruct", "Uops", "res.stl.", "L1D Miss", "CodeMiss"]:
    #     if col_name in filtering_summary:
    #         summary = filtering_summary[col_name]
    #         if summary["original"] > 0:
    #             keep_percent = (summary["filtered"] / summary["original"]) * 100
    #             print(f"{col_name:<12} {summary['original']:>10} {summary['filtered']:>10} "
    #                   f"{summary['removed']:>10} {keep_percent:>9.1f}%")
    #         else:
    #             print(f"{col_name:<12} {summary['original']:>10} {summary['filtered']:>10} "
    #                   f"{summary['removed']:>10} {'N/A':>10}")
    
    # print()
    print("Final statistics (based on double-filtered data):")
    print()
    
    # Calculate statistics for each column using filtered data
    stats_data = []
    header = ["Clock", "Core cyc", "Instruct", "Uops", "res.stl.", "L1D Miss", "CodeMiss"]
    
    for col_name in header:
        if col_name not in filtered_means or not filtered_means[col_name]:
            continue
            
        valid_means = filtered_means[col_name]
        
        if not valid_means:
            continue
        
        mean_val = statistics.mean(valid_means)
        median_val = statistics.median(valid_means)
        variance_val = statistics.variance(valid_means) if len(valid_means) > 1 else 0
        std_dev = statistics.stdev(valid_means) if len(valid_means) > 1 else 0
        min_val = min(valid_means)
        max_val = max(valid_means)
        count = len(valid_means)
        
        stats_data.append([col_name, mean_val, median_val, variance_val, 
                         std_dev, min_val, max_val, count])
    
    # Print table header with proper alignment
    stats_header = f"{'Column':<12} {'Mean':>10} {'Median':>10} {'Variance':>12} {'Std Dev':>10} {'Min':>10} {'Max':>10} {'Count':>8}"
    print(stats_header)
    print("-" * len(stats_header))
    
    # Print data rows with proper alignment
    for row in stats_data:
        col_name = row[0][:11]  # Truncate if too long
        print(f"{col_name:<12} {row[1]:>10.4f} {row[2]:>10.4f} {row[3]:>12.4f} "
              f"{row[4]:>10.4f} {row[5]:>10.4f} {row[6]:>10.4f} {row[7]:>8}")

def main():
    if len(sys.argv) < 3:
        print("Usage: python3 analyze_multi_run.py <results_directory> <num_runs> [second_level_sigma]")
        print("  second_level_sigma: Threshold for second-level filtering (default: 2.0)")
        sys.exit(1)
    
    results_dir = sys.argv[1]
    num_runs = int(sys.argv[2])
    
    # Configuration for second-level filtering
    second_level_sigma = 2.0  # Default threshold for second-level filtering
    if len(sys.argv) > 3:
        try:
            second_level_sigma = float(sys.argv[3])
        except ValueError:
            print(f"Warning: Invalid second_level_sigma '{sys.argv[3]}', using default 2.0")
    
    print(f"Analyzing {num_runs} runs from directory: {results_dir}")
    print(f"First-level selection: Select repetition with median Core cyc from each run")
    print(f"Second-level filtering: {second_level_sigma}σ (filter Core cyc outliers from collection)")
    print()
    
    # Initialize storage for all means
    all_means = {
        "Clock": [],
        "Core cyc": [],
        "Instruct": [],
        "Uops": [],
        "res.stl.": [],
        "L1D Miss": [],
        "CodeMiss": []
    }
    
    # Process each run file
    successful_runs = 0
    failed_runs = 0
    
    for run_num in range(1, num_runs + 1):
        run_file = os.path.join(results_dir, f"run_{run_num}.txt")
        
        if not os.path.exists(run_file):
            failed_runs += 1
            # Add None values for missing runs
            for col_name in all_means:
                all_means[col_name].append(None)
            continue
        
        # Analyze this run (select median repetition)
        selected_values = analyze_single_run(run_file)
        
        if selected_values is None:
            failed_runs += 1
            # Add None values for failed runs
            for col_name in all_means:
                all_means[col_name].append(None)
        else:
            successful_runs += 1
            # Add the selected median values
            for col_name in all_means:
                if col_name in selected_values:
                    all_means[col_name].append(selected_values[col_name])
                else:
                    all_means[col_name].append(None)
    
    print(f"Processing complete:")
    print(f"  Successful runs: {successful_runs}")
    print(f"  Failed runs: {failed_runs}")
    print()
    
    if successful_runs == 0:
        print("No successful runs to analyze!")
        sys.exit(1)
    
    # Apply second-level filtering and create summary table (pass the sigma threshold)
    filtered_means, filtering_summary = apply_second_level_filtering(all_means, sigma_threshold=second_level_sigma)
    create_summary_table_with_double_filtering(all_means, num_runs, second_level_sigma)
    
    # Save detailed results (both single and double filtered)
    output_file = os.path.join(os.path.dirname(results_dir), "filtered_means.txt")
    save_filtered_means(all_means, output_file, filtered_means)
    
    print()
    print(f"Detailed results saved to: {output_file}")
    print("  Contains both unfiltered and Core cyc-filtered data sections")
    
    # # Print summary of filtering effectiveness
    # print()
    # print("=== FILTERING EFFECTIVENESS SUMMARY ===")
    # total_original = sum(filtering_summary[col]["original"] for col in filtering_summary)
    # total_filtered = sum(filtering_summary[col]["filtered"] for col in filtering_summary)
    # total_removed = total_original - total_filtered
    
    # if total_original > 0:
    #     overall_keep_percent = (total_filtered / total_original) * 100
    #     print(f"Overall filtering: {total_filtered}/{total_original} means retained ({overall_keep_percent:.1f}%)")
    #     print(f"Outlier means removed: {total_removed} ({(total_removed/total_original)*100:.1f}%)")
    # else:
    #     print("No data to analyze filtering effectiveness")
    
    # print()
    # print("The final statistics table above is based on Core cyc-filtered data:")
    # print(f"  1st selection: Select repetition with median Core cyc from each run (100 repetitions → 1 selected)")
    # print(f"  2nd filter: Remove data lines with Core cyc outliers beyond {second_level_sigma}σ from collection")
    # print("  Filter application: Core cyc criteria applied uniformly to all columns")
    # print("This ensures clean, representative measurement values with consistent sample sizes.")

if __name__ == "__main__":
    main()
