#include <algorithm>
#include <iostream>
#include <map>
#include <math.h>
#include <set>
#include <string>
#include <utility>
#include <vector>

using namespace std;

const int MAX_RESULT_DOCUMENT_COUNT = 5;

string ReadLine() {
    string s;
    getline(cin, s);
    return s;
}

int ReadLineWithNumber() {
    int result = 0;
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

struct Document {
    int id;
    double relevance;
};

class SearchServer {
public:
    void Print() const {
        for (const auto & [key, m] : documents_) {
            cout<<key<<": ";
            for (const auto & [i, tf]: m )
                cout<<"["<<i<<" "<<tf<<"] ";
            cout<<endl;
        }
    }

    void SetStopWords(const string& text) {
        for (const string& word : SplitIntoWords(text)) {
            stop_words_.insert(word);
        }
    }

    void SetCountDocuments(const int count) { 
        count_documents_ = count;
    }

    void AddDocument(int document_id, const string& document) {
        const vector<string> words = SplitIntoWordsNoStop(document);
        const double size = static_cast <double>(words.size());
        for (const auto& word : words)
        {
            double tf = static_cast<double>(count(words.begin(), words.end(), word))/size; 
            documents_[word].insert({document_id, tf});
        }
    }

    vector<Document> FindTopDocuments(const string& raw_query) const {
        const Query query_words = ParseQuery(raw_query);
        // Print(query_words);

        vector<Document> matched_documents = FindAllDocuments(query_words);

        sort(matched_documents.begin(), matched_documents.end(),
             [](const Document& lhs, const Document& rhs) {
                 return lhs.relevance > rhs.relevance;
             });

        if (matched_documents.size() > MAX_RESULT_DOCUMENT_COUNT) {
            matched_documents.resize(MAX_RESULT_DOCUMENT_COUNT);
        }
        return matched_documents;
    }

private:
    struct Query {
        set<string> minus_words;
        //query, IDF
        map<string, double> plus_words;
    };

    int count_documents_ = 0;
    
    //word, id, TF
    map<string, map<int, double>> documents_;

    set<string> stop_words_;

    bool IsStopWord(const string& word) const {
        return stop_words_.count(word) > 0;
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

    void Print(const Query& q) const {
        for (const auto & str : q.minus_words) {
            cout<<"- "<<str<<endl;
        }
        for (const auto & [str, idf] : q.plus_words) {
            cout<<"+ "<<str<<" idf = "<<idf<<endl;
        }
    }

    Query ParseQuery(const string& text) const {
        Query query;

        for (const string& query_word : SplitIntoWordsNoStop(text)) {
            if (!query_word.empty()) {
                if (query_word[0] == '-') {
                    query.minus_words.insert(query_word.substr(1));
                }
                else {
                    if (documents_.count(query_word)) {
                        double idf = log( static_cast<double>(count_documents_)
                            /static_cast<double>(documents_.at(query_word).size()));

                        query.plus_words.insert({query_word, idf});
                    }
                }               
            }
        }
        return query;
    }

    vector<Document> FindAllDocuments(const Query& query_words) const {
        vector<Document> matched_documents;
        //id_doc, relevance
        map<int, double> relevances;
        
        //находим id по key (слова), счит упоминание
        for (const auto & [plus_word, idf] : query_words.plus_words) {
            if (documents_.count(plus_word)) {
                for (const auto & [id, tf] : documents_.at(plus_word)) {
                    relevances[id] += idf*tf; 
                } 
            }
        }

        for (const auto & minus_word : query_words.minus_words) {
            if (documents_.count(minus_word)) {
                for (const auto & [id, tf] : documents_.at(minus_word)) {
                    if (relevances.count(id)) {
                        relevances.erase(id);
                    }
                }
            }
        }

        for(const auto & [id, relevance] : relevances) {
            matched_documents.push_back({id, relevance});
        }
        return matched_documents;
    }
};

SearchServer CreateSearchServer() {
    SearchServer search_server;
    search_server.SetStopWords(ReadLine());
    
    const int count_documents = ReadLineWithNumber();
    search_server.SetCountDocuments(count_documents);

    for (int document_id = 0; document_id < count_documents; ++document_id) {
        search_server.AddDocument(document_id, ReadLine());
    }

    return search_server;
}

int main() {
    const SearchServer search_server = CreateSearchServer();
// search_server.Print();

    const string query = ReadLine();
    for (const auto& [document_id, relevance] : search_server.FindTopDocuments(query)) {
        cout << "{ document_id = "s << document_id << ", "
             << "relevance = "s << relevance << " }"s << endl;
    }
}