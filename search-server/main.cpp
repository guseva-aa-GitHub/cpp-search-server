#include <algorithm>
#include <cmath>
#include <iostream>
#include <map>
#include <numeric>
#include <set>
#include <string>
#include <vector>

using namespace std;

const int MAX_RESULT_DOCUMENT_COUNT = 5;
const double EPSION = 1e-6;

const int ID_BEGIN_CONTROL_CHAR_FOR_ASCII = 0;
const int ID_END_CONTROL_CHAR_FOR_ASCII = 31;

string ReadLine() {
    string s;
    getline(cin, s);
    return s;
}

int ReadLineWithNumber() {
    int result;
    cin >> result;
    ReadLine();
    return result;
}

vector<string> SplitIntoWords(const string& text) {
    vector<string> words;
    string word;
    for (const char c : text) {
        if (c == ' ') {
            if (!word.empty()) {
                words.push_back(word);
                word.clear();
            }
        } else {
            word += c;
        }
    }
    if (!word.empty()) {
        words.push_back(word);
    }
    return words;
}

void IsNoControlChar(const string & str) {
    for (const char c : str) {
        if (c >= ID_BEGIN_CONTROL_CHAR_FOR_ASCII && c <= ID_END_CONTROL_CHAR_FOR_ASCII)
            throw invalid_argument("IsNoControlChar(), str = "s +str +", not correct."s);
    }
}

struct Document {
    Document() = default;
    Document(const int i, const double rev, int r)
        : id(i), relevance(rev), rating(r) { }

    int id = 0;
    double relevance = 0.;
    int rating = 0;
};

enum class DocumentStatus {
    ACTUAL,
    IRRELEVANT,
    BANNED,
    REMOVED,
};

class SearchServer {
public:
    template <typename StringCollection>
    explicit SearchServer(const StringCollection& stop_words) {
        for (const string& word : stop_words) {
            IsNoControlChar(word);
            if (!word.empty()) {
                stop_words_.insert(word);
            }            
        }
    }

    explicit SearchServer(const string& text) {
        IsNoControlChar(text);
        for (const string& word : SplitIntoWords(text)) {
            stop_words_.insert(word);
        }
    }

    void AddDocument(const int document_id, const string& document, const DocumentStatus status,
                                   const vector<int>& ratings) {
        IsIdForAddDocument(document_id);
        IsNoControlChar(document);

        document_ids_.push_back(document_id);
        const vector<string> words = SplitIntoWordsNoStop(document);
        const double inv_word_count = 1.0 / static_cast<double>(words.size());
        for (const string& word : words) {
            word_to_document_freqs_[word][document_id] += inv_word_count;
        }
        documents_.emplace(document_id, DocumentData{ComputeAverageRating(ratings), status});        
    }

    template <typename DocumentPredicate>
    vector<Document> FindTopDocuments(const string& raw_query, DocumentPredicate document_predicate) const {
        auto query = СorrectQuery(raw_query);        
        vector<Document> matched_documents = FindAllDocuments(query);
        sort(matched_documents.begin(), matched_documents.end(), [](const Document& lhs, const Document& rhs) {
                if (abs(lhs.relevance - rhs.relevance) < EPSION) {
                     return lhs.rating > rhs.rating;
                } 
                return lhs.relevance > rhs.relevance;                 
             });

        if (!matched_documents.empty())
        for (int index = static_cast<int>(matched_documents.size())-1; index >=0; --index ) {
            const int id = matched_documents[index].id;
            if ( !document_predicate(id, documents_.at(id).status, documents_.at(id).rating) ) {
                matched_documents.erase( matched_documents.begin() + index );
            }
        }

        if (static_cast<int>(matched_documents.size()) > MAX_RESULT_DOCUMENT_COUNT) {
            matched_documents.resize(MAX_RESULT_DOCUMENT_COUNT);
        }

        return matched_documents;
    }

    vector<Document> FindTopDocuments(const string& raw_query, const DocumentStatus doc_status) const {
        return FindTopDocuments(raw_query, [doc_status](int document_id, DocumentStatus status, int rating){ 
            return status == doc_status; 
        });
    }

    vector<Document> FindTopDocuments(const string& raw_query) const {
        return FindTopDocuments(raw_query, DocumentStatus::ACTUAL);
    }

    tuple<vector<string>, DocumentStatus> MatchDocument(const string& raw_query, const int document_id) const {       
        auto query = СorrectQuery(raw_query);
        vector<string> query_word_for_document;

        for (const string& word : query.plus_words) {
            if (word_to_document_freqs_.count(word) == 0) {
                continue;
            }

            for (const auto &[id, _] : word_to_document_freqs_.at(word)) {
                if (id == document_id) {
                    query_word_for_document.push_back( word );
                }
            }
        }

        for (const string& word : query.minus_words) {
            if (word_to_document_freqs_.count(word) == 0) {
                continue;
            }
            for (const auto &[id, _] : word_to_document_freqs_.at(word)) {
                if (id == document_id) {
                    query_word_for_document.clear();
                }
            }
        }
        return tuple{query_word_for_document, documents_.at(document_id).status};
    }


    int GetDocumentCount() const {
        return static_cast<int>(documents_.size());
    }

    int GetDocumentId(const int id) const {
        if (id < 0 || id >= static_cast<int>(document_ids_.size())) {
            throw out_of_range("SearchServer::GetDocumentId(), "s + to_string(id) + 
            " out of range [0, "s + to_string(document_ids_.size()) +")."s);
        }
        return document_ids_[id];
    }

private:
    struct DocumentData {
        int rating;
        DocumentStatus status;
    };

    set<string> stop_words_;
    map<string, map<int, double>> word_to_document_freqs_;
    map<int, DocumentData> documents_;
    vector<int> document_ids_;

    bool IsStopWord(const string& word) const {
        return stop_words_.count(word) > 0;
    }

    void IsIdForAddDocument(const int document_id) const {
        if (document_id < 0) {
            throw invalid_argument("SearchServer::IsIdForAddDocument(), document_id = "s 
                                            +to_string(document_id) +" < 0."s);
        }
        for (const int id : document_ids_) {
            if (id == document_id)
            throw invalid_argument("SearchServer::IsIdForAddDocument(), document_id = "s 
                                            +to_string(document_id) +", repeating the index."s);
        }
    }

    vector<string> SplitIntoWordsNoStop(const string& text) const {
        vector<string> words;
        for (const string& word : SplitIntoWords(text)) {
            if (!IsStopWord(word)) {
                words.push_back(word);
            }
        }
        return words;
    }

    static int ComputeAverageRating(const vector<int>& ratings) {
        if (ratings.empty()) {
            return 0;
        }
        return accumulate(ratings.begin(), ratings.end(), 0) / static_cast<int>(ratings.size());
    }

    struct QueryWord {
        QueryWord(const string d, const bool is_m, const bool is_s)
        : data(d), is_minus(is_m), is_stop(is_s){ }

        string data;
        bool is_minus;
        bool is_stop;
    };

    QueryWord ParseQueryWord(string text) const {
        bool is_minus = false;
        // Word shouldn't be empty
        if (text[0] == '-') {
            is_minus = true;
            text = text.substr(1);
        }
        if (is_minus && (text.empty() || (!text.empty() && text[0] == '-'))) {
            throw invalid_argument("SearchServer::ParseQueryWord(), text = "s + text + ", not correct."s);
        }      
        return {text, is_minus, IsStopWord(text)};
    }

    struct Query {
        set<string> plus_words;
        set<string> minus_words;
    };

    Query ParseQuery(const string& text) const {
        Query query;
        for (const string& word : SplitIntoWords(text)) {
            const QueryWord query_word = ParseQueryWord(word);
                          
            if (!query_word.is_stop) {
                if (query_word.is_minus) {
                    query.minus_words.insert(query_word.data);
                } else {
                    query.plus_words.insert(query_word.data);
                }
            }
        }
        return query;
    }

    Query СorrectQuery(const string &raw_query) const {
        IsNoControlChar(raw_query);
        return ParseQuery(raw_query);
    }

    double ComputeWordInverseDocumentFreq(const string& word) const {
        return log(GetDocumentCount() * 1.0 / static_cast<double>(word_to_document_freqs_.at(word).size()));
    }

    vector<Document> FindAllDocuments(const Query& query) const {
        map<int, double> document_to_relevance;
        for (const string& word : query.plus_words) {
            if (word_to_document_freqs_.count(word) == 0) {
                continue;
            }
            const double inverse_document_freq = ComputeWordInverseDocumentFreq(word);
            for (const auto &[document_id, term_freq] : word_to_document_freqs_.at(word)) {
                document_to_relevance[document_id] += term_freq * inverse_document_freq;
            }
        }

        for (const string& word : query.minus_words) {
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
};

void PrintDocument(const Document& document) {
    cout << "{ "s
         << "document_id = "s << document.id << ", "s
         << "relevance = "s << document.relevance << ", "s
         << "rating = "s << document.rating << " }"s << endl;
}

int main() {
    try {
        SearchServer search_server("и в на"s);

        search_server.AddDocument(0, "белый кот и модный ошейник"s, DocumentStatus::ACTUAL, {8, -3});
        search_server.AddDocument(1, "пушистый кот пушистый хвост"s, DocumentStatus::ACTUAL, {7, 2, 7});
        search_server.AddDocument(2, "ухоженный пёс выразительные глаза"s, DocumentStatus::ACTUAL, {5, -12, 2, 1});
        search_server.AddDocument(3, "ухоженный скворец евгений"s, DocumentStatus::BANNED, {9});

        cout << "ACTUAL by default:"s << endl;
        for (const Document& document : search_server.FindTopDocuments("пушистый ухоженный кот"s)) {
            PrintDocument(document);
        }

        cout << "BANNED:"s << endl;
        for (const Document& document : search_server.FindTopDocuments("пушистый ухоженный кот"s, DocumentStatus::BANNED)) {
            PrintDocument(document);
        }

        cout << "Even ids:"s << endl;
        for (const Document &document : search_server.FindTopDocuments("пушистый ухоженный кот"s,
                    [](int document_id, DocumentStatus status, int rating) { return document_id % 2 == 0; }))  
        {
            PrintDocument(document);
        }

        try {
            search_server.GetDocumentId(-5);
        }
        catch (out_of_range & out_error) {
            cout << "Ошибка: "s << out_error.what() << endl;
        }
            
    }
    catch (const invalid_argument& inv) {
        cout << "Ошибка: "s << inv.what() << endl;
    }
}