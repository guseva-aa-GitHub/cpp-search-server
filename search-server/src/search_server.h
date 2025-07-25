#pragma once

#include <algorithm>
#include <map>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>

#include "document.h"
#include "string_processing.h"

enum class DocumentStatus {
    ACTUAL,
    IRRELEVANT,
    BANNED,
    REMOVED,
};

using std::map;
using std::set;
using std::string;
using std::vector;
using std::tuple;

constexpr double EPSION = 1e-6;
const int MAX_RESULT_DOCUMENT_COUNT = 5;

class SearchServer {
public:
    template <typename StringContainer>
    explicit SearchServer(const StringContainer& stop_words);
    explicit SearchServer(const string& stop_words_text);

    void AddDocument(int document_id, const string& document, DocumentStatus status,
            const std::vector<int>& ratings);

    template <typename DocumentPredicate>
    vector<Document> FindTopDocuments(const string& raw_query, DocumentPredicate document_predicate) const;
    vector<Document> FindTopDocuments(const string& raw_query, DocumentStatus status) const;
    vector<Document> FindTopDocuments(const string& raw_query) const;

    int GetDocumentCount() const;
    int GetDocumentId(int index) const;

    tuple<vector<string>, DocumentStatus> MatchDocument(const string& raw_query,
                                                        int document_id) const;

private:
    struct DocumentData {
        int rating;
        DocumentStatus status;
    };

    const set<string> stop_words_;
    map<string, map<int, double>> word_to_document_freqs_;
    map<int, DocumentData> documents_;
    vector<int> document_ids_;

    bool IsStopWord(const string& word) const;
    static bool IsValidWord(const string& word);
    vector<string> SplitIntoWordsNoStop(const string& text) const;
    static int ComputeAverageRating(const vector<int>& ratings);

    struct QueryWord {
        string data;
        bool is_minus;
        bool is_stop;
    };
    QueryWord ParseQueryWord(const string& text) const;

    struct Query {
        set<string> plus_words;
        set<string> minus_words;
    };
    Query ParseQuery(const string& text) const;
    
    double ComputeWordInverseDocumentFreq(const string& word) const;

    template <typename DocumentPredicate>
    vector<Document> FindAllDocuments(const Query& query, DocumentPredicate document_predicate) const; 
};

template <typename StringContainer>
SearchServer::SearchServer(const StringContainer& stop_words)
        : stop_words_(MakeUniqueNonEmptyStrings(stop_words)) {
    if (!all_of(stop_words_.begin(), stop_words_.end(), IsValidWord)) {
        throw std::invalid_argument(string{"Some of stop words are invalid"});
    }
}

template <typename DocumentPredicate>
vector<Document> SearchServer::FindTopDocuments(const string& raw_query,
                                  DocumentPredicate document_predicate) const {
    const auto query = ParseQuery(raw_query);
    auto matched_documents = FindAllDocuments(query, document_predicate);

    sort(matched_documents.begin(), matched_documents.end(),
         [](const Document &lhs, const Document &rhs) {
             if (std::abs(lhs.relevance - rhs.relevance) < EPSION) {
                return lhs.rating > rhs.rating;
             } else {
                return lhs.relevance > rhs.relevance;
             }
         });

    if (matched_documents.size() > MAX_RESULT_DOCUMENT_COUNT) {
        matched_documents.resize(MAX_RESULT_DOCUMENT_COUNT);
    }

    return matched_documents;
}

template <typename DocumentPredicate>
vector<Document> SearchServer::FindAllDocuments(const Query &query,
                                  DocumentPredicate document_predicate) const {
    map<int, double> document_to_relevance;
    for (const string &word : query.plus_words) {
        if (word_to_document_freqs_.count(word) == 0) {
            continue;
        }
        const double inverse_document_freq = ComputeWordInverseDocumentFreq(word);
        for (const auto &[document_id, term_freq] : word_to_document_freqs_.at(word)) {
            const auto &document_data = documents_.at(document_id);
            if (document_predicate(document_id, document_data.status, document_data.rating)) {
                document_to_relevance[document_id] += term_freq * inverse_document_freq;
            }
        }
    }

    for (const string &word : query.minus_words) {
        if (word_to_document_freqs_.count(word) == 0) {
            continue;
        }
        for (const auto &[document_id, _] : word_to_document_freqs_.at(word)) {
            document_to_relevance.erase(document_id);
        }
    }

    vector<Document> matched_documents;
    for (const auto &[document_id, relevance] : document_to_relevance) {
        matched_documents.push_back(
            {document_id, relevance, documents_.at(document_id).rating});
    }
    return matched_documents;
}

void AddDocument(SearchServer& search_server, int document_id, const string& document,
                 DocumentStatus status, const vector<int>& ratings);

void FindTopDocuments(const SearchServer& search_server, const string& raw_query);

void MatchDocuments(const SearchServer& search_server, const string& query);

void PrintMatchDocumentResult(int document_id, const vector<string>& words, DocumentStatus status);
