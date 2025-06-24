#include "cppjieba/Jieba.hpp"
#include <regex> 
// #include "cppjieba/PosTagger.hpp" 
// #include "limonp/Logging.hpp"
// #include <iostream>
// #include <fstream>
// #include <vector>
// #include <map>
// #include <set>
// #include <algorithm>
// #include <cmath>
// #include <unordered_set>
// #include <unordered_map>

// 情感分析关键词
const std::set<std::string> POSITIVE_WORDS = {"幸福", "快乐", "成功", "胜利", "希望", "爱", "和平", "光明", "团圆", "喜悦", "赞美", "美好", "积极", "鼓励", "进步"};
const std::set<std::string> NEGATIVE_WORDS = {"死亡", "悲伤", "失败", "痛苦", "仇恨", "战争", "黑暗", "绝望", "背叛", "恐惧", "不幸", "消极", "抱怨", "退缩", "困难"};

// 故事类型关键词
const std::map<std::string, std::set<std::string>> GENRE_KEYWORDS = {
    {"爱情", {"爱", "恋人", "结婚", "感情", "亲吻", "浪漫", "心动", "约会", "相遇", "分手", "思念", "情人"}},
    {"奇幻", {"魔法", "龙", "精灵", "巫师", "咒语", "城堡", "幻想", "异世界", "神", "妖", "仙", "魔", "传说", "神器"}},
    {"悬疑", {"谋杀", "侦探", "线索", "秘密", "真相", "凶手", "犯罪", "解谜", "疑团", "调查", "案件", "推理", "陷阱"}},
    {"科幻", {"宇宙", "飞船", "激光", "未来", "机器人", "人工智能", "外星人", "科技", "星际", "基因", "虚拟", "改造", "超能力"}},
    {"历史", {"皇帝", "将军", "战争", "朝代", "古装", "历史", "宫廷", "文物", "王朝", "民族", "叛乱", "征战", "英雄"}}
};

// 结局预测关键词
const std::set<std::string> GOOD_ENDING_WORDS = {"幸福", "团圆", "和平", "胜利", "成功", "光明", "希望", "美满", "圆满", "新生", "曙光"};
const std::set<std::string> BAD_ENDING_WORDS = {"死亡", "失败", "绝望", "结束", "毁灭", "牺牲", "悲剧", "消亡", "破碎", "分离", "不幸"};

// 常见姓氏
const std::unordered_set<std::string> COMMON_SURNAMES = {
    "赵", "钱", "孙", "李", "周", "吴", "郑", "王", "冯", "陈", "褚", "卫", "蒋", "沈", "韩", "杨", "朱", "秦", "尤", "许", "何", "吕", "施", "张", "孔", "曹", "严", "华", "金", "魏", "陶", "姜", "戚", "谢", "邹", "喻", "柏", "水", "窦", "章", "云", "苏", "潘", "葛", "奚", "范", "彭", "郎", "鲁", "韦", "昌", "马", "苗", "凤", "花", "方", "俞", "任", "袁", "柳", "丰", "鲍", "史", "唐", "费", "廉", "岑", "薛", "雷", "贺", "倪", "汤", "滕", "殷", "罗", "毕", "郝", "邬", "安", "常", "乐", "于", "时", "傅", "皮", "卞", "齐", "康", "伍", "余", "元", "卜", "顾", "孟", "平", "黄", "和", "穆", "萧", "尹", "姚", "邵", "湛", "汪", "祁", "毛", "禹", "狄", "米", "贝", "明", "臧", "计", "伏", "成", "戴", "谈", "宋", "茅", "庞", "熊", "纪", "舒", "屈", "项", "祝", "董", "梁", "杜", "阮", "蓝", "闵", "席", "季", "麻", "强", "贾", "路", "娄", "危", "江", "童", "颜", "郭", "梅", "盛", "林", "刁", "钟", "徐", "丘", "骆", "高", "夏", "蔡", "田", "樊", "胡", "凌", "霍", "虞", "万", "支", "柯", "昝", "管", "卢", "莫", "经", "房", "裘", "缪", "干", "解", "应", "宗", "丁", "宣", "贲", "邓", "郁", "单", "杭", "洪", "包", "诸", "左", "石", "崔", "吉", "钮", "龚", "程", "嵇", "邢", "滑", "裴", "陆", "荣", "翁", "荀", "羊", "於", "惠", "甄", "麹", "家", "封", "芮", "羿", "储", "靳", "汲", "邴", "糜", "松", "井", "段", "富", "巫", "乌", "焦", "巴", "弓", "牧", "隗", "山", "谷", "车", "侯", "宓", "蓬", "全", "郗", "班", "仰", "秋", "仲", "伊", "宫", "宁", "仇", "栾", "暴", "甘", "钭", "厉", "戎", "祖", "武", "符", "刘", "景", "詹", "束", "龙", "叶", "幸", "司", "韶", "郜", "黎", "蓟", "薄", "印", "宿", "白", "怀", "蒲", "邰", "从", "鄂", "索", "咸", "籍", "赖", "卓", "蔺", "屠", "蒙", "池", "乔", "阴", "郁", "胥", "能", "苍", "双", "闻", "莘", "党", "翟", "谭", "贡", "劳", "逄", "姬", "申", "扶", "堵", "冉", "宰", "郦", "雍", "郤", "璩", "桑", "桂", "濮", "牛", "寿", "通", "边", "扈", "燕", "冀", "郏", "浦", "尚", "农", "温", "别", "庄", "晏", "柴", "瞿", "阎", "充", "慕", "连", "茹", "习", "宦", "艾", "鱼", "容", "向", "古", "易", "慎", "戈", "廖", "庾", "终", "暨", "居", "衡", "步", "都", "耿", "满", "弘", "匡", "国", "文", "寇", "广", "禄", "阙", "东", "欧", "殳", "沃", "利", "蔚", "越", "夔", "隆", "师", "巩", "厙", "聂", "晁", "勾", "敖", "融", "冷", "訾", "辛", "阚", "那", "简", "饶", "空", "曾", "毋", "沙", "乜", "养", "鞠", "须", "丰", "巢", "关", "蒯", "相", "查", "后", "荆", "红", "游", "竺", "权", "逯", "盖", "益", "桓", "公", "万俟", "司马", "上官", "欧阳", "夏侯", "诸葛", "闻人", "东方", "赫连", "皇甫", "尉迟", "公羊", "澹台", "公冶", "宗政", "濮阳", "淳于", "单于", "太叔", "申屠", "公孙", "仲孙", "轩辕", "令狐", "钟离", "宇文", "长孙", "慕容", "司徒", "司空"
};

// 动词词性集合
const std::set<std::string> VERB_POS_TAGS = {
    "v", "vd", "vn", "vshi", "vyou", "vf", "vx", "vi", "vl", "vg"
};

// 参数
struct AnalysisConfig {
    int minNameLength = 2;    // 最小名字长度
    int maxNameLength = 4;     // 最大名字长度
    int minNameScore = 15;    // 最低名字得分
    int minNameOccurrence = 3; // 最小出现次数
    double endingRatio = 2.0; // 结局判定比例
    int minEndingCount = 5;    // 最小结局关键词数量
};

// 全局变量
std::unordered_set<std::string> STOP_WORDS;
std::unordered_set<std::string> NAME_FILTER;
AnalysisConfig config;

// 特殊符号正则表达式
const static std::regex CH_PUNCT_REGEX(u8"[“”‘’，。？！；：、《》（）【】—…—·]");

// 加载过滤器
void loadFilter(const std::string& filePath, std::unordered_set<std::string>& filter) {
    std::ifstream ifs(filePath);
    if (!ifs.is_open()) {
        XLOG(ERROR) << "Error opening filter file: " << filePath;
        return;
    }
    std::string line;
    while (std::getline(ifs, line)) {
        if (!line.empty()) {
            filter.insert(std::move(line));
        }
    }
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

// 加载词典
void loadAllDictionaries() {
    auto load = [](const std::string& path, auto& container) {
        std::ifstream ifs(path);
        if (!ifs) {
            std::cerr << "Error opening dictionary: " << path << std::endl;
            return;
        }
        std::string line;
        while (std::getline(ifs, line)) {
            if (!line.empty()) {
                container.insert(line);
            }
        }
    };
    
    load("../dict/stop_words.utf8", STOP_WORDS);
    load("../dict/name_filter.utf8", NAME_FILTER);
}

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <novel_file> [output_file]" << std::endl;
        return 1;
    }

    // 加载词典
    loadAllDictionaries();

    cppjieba::Jieba jieba("../dict/jieba.dict.utf8",
                          "../dict/hmm_model.utf8",
                          "../dict/user.dict.utf8",
                          "../dict/idf.utf8",
                          "../dict/stop_words.utf8");

    // 读取小说
    std::ifstream file(argv[1]);
    if (!file) {
        std::cerr << "Error opening file: " << argv[1] << std::endl;
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
    tokens.reserve(jieba_tokens.size()); // 预分配内存
    for (const auto& token : jieba_tokens) {
        tokens.push_back({token.first, token.second});
    }
    
    // 提取，过滤词语
    std::vector<std::string> filteredWords;
    filteredWords.reserve(tokens.size()); // 预分配内存
    
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

    // 输出
    std::ostream* out = &std::cout;
    std::ofstream outFile;

    if (argc > 2) {
        outFile.open(argv[2]);
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