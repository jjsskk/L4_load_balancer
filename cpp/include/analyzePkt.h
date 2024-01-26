#ifndef ANALYZE_H
#define ANALYZE_H
void analyzeIPDatagram(char *buffer, int size);

void analyzeTCPSegment(char *buffer, int size);
#endif