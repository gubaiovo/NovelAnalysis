#include "cppjieba/Jieba.hpp"
#include <regex> 
#include <fstream>
#include <sstream>
#include <iostream>
#include <unordered_map>
#include <vector>
#include <set>
#include <map>
#include <algorithm>
#include <unordered_set>
#include <filesystem>

struct AppConfig {
    // 字典
    std::string jiebaDictPath = "./dict/jieba/jieba.dict.utf8"; 
    std::string hmmModelPath = "./dict/jieba/hmm_model.utf8";
    std::string userDictPath = "./dict/jieba/user.dict.utf8";
    std::string idfPath = "./dict/jieba/idf.utf8";
    std::string stopWordsPath = "./dict/jieba/stop_words.utf8";
    std::string nameFilterPath = "./dict/name_filter.utf8";
    
    // 关键词
    std::string positiveWordsPath = "./dict/positive_words.utf8";
    std::string negativeWordsPath = "./dict/negative_words.utf8";
    std::string goodEndingWordsPath = "./dict/good_ending_words.utf8";
    std::string badEndingWordsPath = "./dict/bad_ending_words.utf8";
    std::string surnamesPath = "./dict/surnames.utf8";
    std::string verbTagsPath = "./dict/verb_tags.utf8";
    std::string genreKeywordsDir = "./dict/genre_keywords/";
    
    // 分析参数
    struct AnalysisConfig {
        int minNameLength = 2;
        int maxNameLength = 4;
        int minNameScore = 15;
        int minNameOccurrence = 3;
        double endingRatio = 2.0;
        int minEndingCount = 5;
    } analysisConfig;
};


AppConfig appConfig;
std::unordered_set<std::string> STOP_WORDS;
std::unordered_set<std::string> NAME_FILTER;
std::set<std::string> POSITIVE_WORDS;
std::set<std::string> NEGATIVE_WORDS;
std::set<std::string> GOOD_ENDING_WORDS;
std::set<std::string> BAD_ENDING_WORDS;
std::unordered_set<std::string> COMMON_SURNAMES;
std::set<std::string> VERB_POS_TAGS;
std::map<std::string, std::set<std::string>> GENRE_KEYWORDS;

const static std::regex CH_PUNCT_REGEX(u8"[“”‘’，。？！；：、《》（）【】—…—·]");

// 加载词典
void loadDictionary(const std::string& filePath, std::set<std::string>& container) {
    std::ifstream ifs(filePath);
    if (!ifs.is_open()) {
        std::cerr << "Error opening dictionary: " << filePath << std::endl;
        return;
    }
    std::string line;
    while (std::getline(ifs, line)) {
        if (!line.empty()) {
            container.insert(line);
        }
    }
}

// 加载词典到unordered_set
void loadDictionary(const std::string& filePath, std::unordered_set<std::string>& container) {
    std::ifstream ifs(filePath);
    if (!ifs.is_open()) {
        std::cerr << "Error opening dictionary: " << filePath << std::endl;
        return;
    }
    std::string line;
    while (std::getline(ifs, line)) {
        if (!line.empty()) {
            container.insert(line);
        }
    }
}

// 加载故事类型关键词
void loadGenreKeywords(const std::string& dirPath) {
    namespace fs = std::filesystem;
    
    try {
        for (const auto& entry : fs::directory_iterator(dirPath)) {
            if (entry.is_regular_file()) {
                // 获取文件名作为类型名
                std::string genre = entry.path().stem().string();
                
                // 加载关键词
                std::set<std::string> keywords;
                loadDictionary(entry.path().string(), keywords);
                
                GENRE_KEYWORDS[genre] = keywords;
            }
        }
    } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "Error loading genre keywords: " << e.what() << std::endl;
    }
}

// 加载所有词典
void loadAllDictionaries() {
    // 加载停用词和名称过滤器
    loadDictionary(appConfig.stopWordsPath, STOP_WORDS);
    loadDictionary(appConfig.nameFilterPath, NAME_FILTER);
    
    // 加载情感分析关键词
    loadDictionary(appConfig.positiveWordsPath, POSITIVE_WORDS);
    loadDictionary(appConfig.negativeWordsPath, NEGATIVE_WORDS);
    
    // 加载结局关键词
    loadDictionary(appConfig.goodEndingWordsPath, GOOD_ENDING_WORDS);
    loadDictionary(appConfig.badEndingWordsPath, BAD_ENDING_WORDS);
    
    // 加载姓氏
    loadDictionary(appConfig.surnamesPath, COMMON_SURNAMES);
    
    // 加载动词词性标签
    loadDictionary(appConfig.verbTagsPath, VERB_POS_TAGS);
    
    // 加载故事类型关键词
    loadGenreKeywords(appConfig.genreKeywordsDir);
}

// 解析配置文件
bool parseConfig(const std::string& configPath) {
    std::ifstream configFile(configPath);
    if (!configFile.is_open()) {
        std::cerr << "Error opening config file: " << configPath << std::endl;
        return false;
    }

    std::string line;
    std::string currentSection;
    while (std::getline(configFile, line)) {
        // 移除行首行尾空白
        line.erase(0, line.find_first_not_of(" \t"));
        line.erase(line.find_last_not_of(" \t") + 1);
        
        // 跳过空行和注释
        if (line.empty() || line[0] == '#' || line[0] == ';') {
            continue;
        }
        
        // 处理节头
        if (line[0] == '[' && line[line.size()-1] == ']') {
            currentSection = line.substr(1, line.size()-2);
            continue;
        }
        
        // 分割键值对
        size_t pos = line.find('=');
        if (pos == std::string::npos) continue;
        
        std::string key = line.substr(0, pos);
        std::string value = line.substr(pos + 1);
        
        // 移除键值对两端的空白
        key.erase(0, key.find_first_not_of(" \t"));
        key.erase(key.find_last_not_of(" \t") + 1);
        value.erase(0, value.find_first_not_of(" \t"));
        value.erase(value.find_last_not_of(" \t") + 1);
        
        // 处理不同节的配置
        if (currentSection == "Dictionaries") {
            if (key == "jieba_dict") appConfig.jiebaDictPath = value;
            else if (key == "hmm_model") appConfig.hmmModelPath = value;
            else if (key == "user_dict") appConfig.userDictPath = value;
            else if (key == "idf") appConfig.idfPath = value;
            else if (key == "stop_words") appConfig.stopWordsPath = value;
            else if (key == "name_filter") appConfig.nameFilterPath = value;
            else if (key == "positive_words") appConfig.positiveWordsPath = value;
            else if (key == "negative_words") appConfig.negativeWordsPath = value;
            else if (key == "good_ending_words") appConfig.goodEndingWordsPath = value;
            else if (key == "bad_ending_words") appConfig.badEndingWordsPath = value;
            else if (key == "surnames") appConfig.surnamesPath = value;
            else if (key == "verb_tags") appConfig.verbTagsPath = value;
            else if (key == "genre_keywords_dir") appConfig.genreKeywordsDir = value;
        }
        else if (currentSection == "AnalysisConfig") {
            if (key == "min_name_length") appConfig.analysisConfig.minNameLength = std::stoi(value);
            else if (key == "max_name_length") appConfig.analysisConfig.maxNameLength = std::stoi(value);
            else if (key == "min_name_score") appConfig.analysisConfig.minNameScore = std::stoi(value);
            else if (key == "min_name_occurrence") appConfig.analysisConfig.minNameOccurrence = std::stoi(value);
            else if (key == "ending_ratio") appConfig.analysisConfig.endingRatio = std::stod(value);
            else if (key == "min_ending_count") appConfig.analysisConfig.minEndingCount = std::stoi(value);
        }
    }
    
    return true;
}

// 检查是否是停用词
bool isStopWord(const std::string& word) {
    return STOP_WORDS.find(word) != STOP_WORDS.end();
}

// 结构体用于存储分词和词性结果
struct Token {
    std::string word;
    std::string tag;
};

// 检查是否包含常见姓氏
bool hasCommonSurname(std::string_view word) {
    if (word.length() >= 6) {
        std::string prefix2(word.substr(0, 6));
        if (COMMON_SURNAMES.find(prefix2) != COMMON_SURNAMES.end()) {
            return true;
        }
    }
    if (word.length() >= 3) {
        std::string prefix1(word.substr(0, 3));
        if (COMMON_SURNAMES.find(prefix1) != COMMON_SURNAMES.end()) {
            return true;
        }
    }
    return false;
}

void analyzeCharacters(const std::vector<Token>& tokens,
                      std::unordered_map<std::string, int>& characterCount) {
    // 候选人物
    std::unordered_map<std::string, int> candidates;
    std::unordered_map<std::string, int> freqMap; // 频率统计
    
    const auto& config = appConfig.analysisConfig;
    
    // 1. 基础筛选和频率统计
    for (const auto& token : tokens) {
        const auto& word = token.word;
        
        // 基本过滤条件
        if (word.size() < config.minNameLength * 3 || 
            word.size() > config.maxNameLength * 3) continue; // UTF-8: 每个汉字3字节
        if (NAME_FILTER.find(word) != NAME_FILTER.end()) continue;
        if (VERB_POS_TAGS.find(token.tag) != VERB_POS_TAGS.end()) continue;
        
        // 姓氏匹配
        if (hasCommonSurname(word)) {
            candidates[word] += 3; // 基础权重
        }
        freqMap[word]++; // 频率统计
    }
    
    // 2. 上下文分析
    for (size_t i = 0; i < tokens.size(); ++i) {
        const auto& word = tokens[i].word;
        
        // 跳过非候选词
        if (candidates.find(word) == candidates.end()) continue;
        
        // 对话模式: [人物]说："..."
        if (i >= 2) {
            const auto& prevWord = tokens[i-1].word;
            const auto& prevPrevWord = tokens[i-2].word;
            
            if ((prevWord == "说" || prevWord == "道" || prevWord == "问") && 
                tokens[i].word == "：") {
                if (hasCommonSurname(prevPrevWord)) {
                    candidates[prevPrevWord] += 8; // 高权重
                }
            }
        }
        
        // 称谓模式: [人物]先生/女士
        if (i < tokens.size() - 1) {
            const auto& nextWord = tokens[i+1].word;
            if (nextWord == "先生" || nextWord == "女士" || nextWord == "小姐" ||
                nextWord == "夫人" || nextWord == "老师" || nextWord == "博士") {
                candidates[word] += 5;
            }
        }
        
        // 动词关系分析
        if (i < tokens.size() - 1) {
            const auto& nextWord = tokens[i+1].word;
            if (VERB_POS_TAGS.find(tokens[i+1].tag) != VERB_POS_TAGS.end()) {
                candidates[word] += 2;
            }
        }
    }
    
    // 3. 最终过滤和评分
    const int minOccurrence = std::max(config.minNameOccurrence, 
                                      static_cast<int>(tokens.size() / 5000));
    
    for (const auto& [name, score] : candidates) {
        int freq = freqMap[name];
        
        // 应用过滤标准
        if (score >= config.minNameScore && freq >= minOccurrence) {
            // 最终得分 = 上下文得分 + 频率分（上限20）
            characterCount[name] = score + std::min(freq, 20);
        }
    }
}

// 情感、类型和结局分析
void combinedAnalysis(const std::vector<std::string>& words, 
                      double& sentiment, 
                      std::string& genre, 
                      std::string& ending) {
    int positive = 0;
    int negative = 0;
    std::map<std::string, int> genreScores;
    int goodEnding = 0;
    int badEnding = 0;

    for (const auto& word : words) {
        // 情感分析
        if (POSITIVE_WORDS.find(word) != POSITIVE_WORDS.end()) {
            positive++;
        } else if (NEGATIVE_WORDS.find(word) != NEGATIVE_WORDS.end()) {
            negative++;
        }
        
        // 类型分析
        for (const auto& [genreKey, keywords] : GENRE_KEYWORDS) {
            if (keywords.find(word) != keywords.end()) {
                genreScores[genreKey]++;
            }
        }
        
        // 结局分析
        if (GOOD_ENDING_WORDS.find(word) != GOOD_ENDING_WORDS.end()) {
            goodEnding++;
        } else if (BAD_ENDING_WORDS.find(word) != BAD_ENDING_WORDS.end()) {
            badEnding++;
        }
    }

    // 情感倾向
    if (positive + negative == 0) {
        sentiment = 0.0;
    } else {
        sentiment = static_cast<double>(positive - negative) / (positive + negative);
    }

    // 故事类型
    if (genreScores.empty()) {
        genre = "未知";
    } else {
        auto maxGenre = std::max_element(genreScores.begin(), genreScores.end(),
            [](const auto& a, const auto& b) { return a.second < b.second; });
        genre = maxGenre->first;
    }

    // 预测结局
    const auto& config = appConfig.analysisConfig;
    if (goodEnding > badEnding * config.endingRatio && goodEnding > config.minEndingCount) {
        ending = "美好结局";
    } else if (badEnding > goodEnding * config.endingRatio && badEnding > config.minEndingCount) {
        ending = "悲剧结局";
    } else if (std::abs(goodEnding - badEnding) <= 2 && (goodEnding + badEnding) > 0) {
        ending = "开放式结局";
    } else {
        ending = "中性结局";
    }
}

int main(int argc, char** argv) {
    // 默认配置文件路径
    std::string configPath = "config.ini";
    
    if (argc > 1 && std::string(argv[1]) == "-c" && argc >= 3) {
        configPath = argv[2];
        for (int i = 2; i < argc - 1; i++) {
            argv[i] = argv[i+1];
        }
        argc--;
    }

    if (!std::filesystem::exists(configPath)) {
        std::cerr << "Config file not found: " << configPath << std::endl;
        std::cout << "Creating default config file..." << std::endl;
        std::ofstream configFile(configPath);
        if (configFile.is_open()) {
            configFile << "[Dictionaries]" << std::endl;
            configFile << "jieba_dict = ./dict/jieba/jieba.dict.utf8" << std::endl;
            configFile << "hmm_model = ./dict/jieba/hmm_model.utf8" << std::endl;
            configFile << "user_dict = ./dict/jieba/user.dict.utf8" << std::endl;
            configFile << "idf = ./dict/jieba/idf.utf8" << std::endl;
            configFile << "stop_words = ./dict/jieba/stop_words.utf8" << std::endl;
            configFile << "name_filter = ./dict/name_filter.utf8" << std::endl;
            configFile << "positive_words = ./dict/positive_words.utf8" << std::endl;
            configFile << "negative_words = ./dict/negative_words.utf8" << std::endl;
            configFile << "good_ending_words = ./dict/good_ending_words.utf8" << std::endl;
            configFile << "bad_ending_words = ./dict/bad_ending_words.utf8" << std::endl;
            configFile << "surnames = ./dict/surnames.utf8" << std::endl;
            configFile << "verb_tags = ./dict/verb_tags.utf8" << std::endl;
            configFile << "genre_keywords_dir = ./dict/genre_keywords/" << std::endl;
            configFile << std::endl;
            configFile << "[AnalysisConfig]" << std::endl;
            configFile << "min_name_length = 2" << std::endl;
            configFile << "max_name_length = 4" << std::endl;
            configFile << "min_name_score = 15" << std::endl;
            configFile << "min_name_occurrence = 3" << std::endl;
            configFile << "ending_ratio = 2.0" << std::endl;
            configFile << "min_ending_count = 5" << std::endl;
            configFile.close();
            std::cout << "Default config file created at: " << configPath << std::endl;
        } else {
            std::cerr << "Error creating default config file: " << configPath << std::endl;
            return 1;
        }
    
        return 1;
    }

    if (!parseConfig(configPath)) {
        std::cerr << "Using default configuration due to config file error" << std::endl;
    }

    loadAllDictionaries();

    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " [-c config_file] <novel_file> [output_file]" << std::endl;
        return 1;
    }

    std::string novelFile = argv[1];
    std::string outputFile = (argc > 2) ? argv[2] : "";

    cppjieba::Jieba jieba(appConfig.jiebaDictPath,
                          appConfig.hmmModelPath,
                          appConfig.userDictPath,
                          appConfig.idfPath,
                          appConfig.stopWordsPath);

    std::ifstream file(novelFile);
    if (!file) {
        std::cerr << "Error opening file: " << novelFile << std::endl;
        return 1;
    }

    std::string text((std::istreambuf_iterator<char>(file)),
                     std::istreambuf_iterator<char>());
    file.close();

    // 分词，词性标注
    std::vector<std::pair<std::string, std::string>> jieba_tokens;
    jieba.Tag(text, jieba_tokens);
    
    // 转换为Token
    std::vector<Token> tokens;
    tokens.reserve(jieba_tokens.size());
    for (const auto& token : jieba_tokens) {
        tokens.push_back({token.first, token.second});
    }
    
    // 提取，过滤词语
    std::vector<std::string> filteredWords;
    filteredWords.reserve(tokens.size());
    
    for (const auto& token : tokens) {
        const auto& word = token.word;
        
        // 过滤条件
        if (!isStopWord(word) && 
            word.length() > 3 &&  // UTF-8下至少1个汉字(3字节)
            !std::all_of(word.begin(), word.end(), ::isspace) &&
            !std::regex_match(word, CH_PUNCT_REGEX)) {
            filteredWords.push_back(word);
        }
    }

    // 基本统计
    int wordCount = filteredWords.size();
    std::unordered_set<std::string> uniqueWords(filteredWords.begin(), filteredWords.end());
    int uniqueCount = uniqueWords.size();

    // 词频统计
    std::unordered_map<std::string, int> wordFreq;
    for (const auto& word : filteredWords) {
        wordFreq[word]++;
    }

    // 前20个高频词
    std::vector<std::pair<std::string, int>> topWords(wordFreq.begin(), wordFreq.end());
    auto endIter = topWords.size() > 20 ? topWords.begin() + 20 : topWords.end();
    std::partial_sort(topWords.begin(), endIter, topWords.end(),
        [](const auto& a, const auto& b) { return a.second > b.second; });

    // 人物统计
    std::unordered_map<std::string, int> characterCount;
    analyzeCharacters(tokens, characterCount);

    // 找出前10个高频人物
    std::vector<std::pair<std::string, int>> topCharacters(characterCount.begin(), characterCount.end());
    std::sort(topCharacters.begin(), topCharacters.end(),
        [](const auto& a, const auto& b) { return a.second > b.second; });
    if (topCharacters.size() > 10) {
        topCharacters.resize(10);
    }

    // 合并分析
    double sentiment;
    std::string genre, ending;
    combinedAnalysis(filteredWords, sentiment, genre, ending);

    std::ostream* out = &std::cout;
    std::ofstream outFile;

    if (!outputFile.empty()) {
        outFile.open(outputFile);
        if (outFile.is_open()) {
            out = &outFile;
        }
    }

    *out << "===== 小说分析报告 =====" << std::endl;
    *out << "总词数 (过滤后): " << wordCount << std::endl;
    *out << "唯一词数 (过滤后): " << uniqueCount << std::endl;
    *out << "词汇丰富度: " << (wordCount > 0 ? static_cast<double>(uniqueCount) / wordCount * 100 : 0.0) << "%" << std::endl;
    *out << "情感倾向: " << sentiment << " ("
              << (sentiment > 0.1 ? "积极" : sentiment < -0.1 ? "消极" : "中性") << ")" << std::endl;
    *out << "预测故事类型: " << genre << std::endl;
    *out << "预测结局: " << ending << std::endl;

    *out << "\n===== 高频词 Top 20 =====" << std::endl;
    for (size_t i = 0; i < std::min(size_t(20), topWords.size()); ++i) {
        *out << topWords[i].first << ": " << topWords[i].second << std::endl;
    }

    *out << "\n===== 人物统计 Top 10 =====" << std::endl;
    for (size_t i = 0; i < topCharacters.size(); ++i) {
        *out << topCharacters[i].first << ": " << topCharacters[i].second << std::endl;
    }

    *out << "\n===== 关键指标 =====" << std::endl;
    *out << "积极词数量: " << std::count_if(filteredWords.begin(), filteredWords.end(),
              [](const std::string& w) { return POSITIVE_WORDS.count(w); }) << std::endl;
    *out << "消极词数量: " << std::count_if(filteredWords.begin(), filteredWords.end(),
              [](const std::string& w) { return NEGATIVE_WORDS.count(w); }) << std::endl;

    if (outFile.is_open()) {
        outFile.close();
    }

    return 0;
}