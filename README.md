# Log Parser - Illumio Assessment

## Brief Overview
This project processes flow log data and maps each row to a tag by referring to a lookup table provided as a CSV file. The lookup table contains destination port (`dstport`), protocol, and assigns a tag to each combination. The program generates an output file containing:
- Count of matches for each tag
- Count of matches for each port/protocol combination

## Files
- **sampleLogs.txt**: Input file containing flow log entries.
- **sampleLookup.csv**: A CSV file with columns for destination port, protocol, and tag.
- **logParser.cpp**: The main C++ source code file.

## Commands to Run the Program
```sh
# Compile the program
g++ -std=c++17 -o logParser logParser.cpp

# Execute the program
./logParser sampleLogs.txt sampleLookup.csv output.txt
```
The output will be generated in `output.txt`.

## Approach
1. **Parsing the Lookup Table:**
   - The lookup table (CSV file) is read and stored in an unordered map.
   - Each entry consists of a destination port and protocol mapped to a corresponding tag.

2. **Processing the Log File:**
   - The log file is read line after line.
   - We have to extract relevant fields such as destination port, protocol, action, and status for which I used regex pattern.
   - The TCP, UDP, ICMP etc protocols are being extracted using protocol number.
   - Only logs with `ACCEPT` action and `OK` status are considered.
   - A count is maintained for each tag and each port/protocol combination.
   - Entries without a matching tag are counted separately as "Untagged."

3. **Generating the Output File:**
   - The results are sorted and formatted into a structured output file.
   - The count of each tag and port/protocol combination is being written to the output file in CSV format.

## Conclusion
This log parser effectively processes flow log data and maps it to predefined tags based on a lookup table. By filtering only accepted and valid log entries, it ensures accuracy in generating insights. The structured output format makes it easy to analyze and interpret the results. The program is efficient and scalable, making it suitable for processing large log files in a real-world scenario.

