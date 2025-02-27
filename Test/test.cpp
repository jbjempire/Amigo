#include "pch.h"

#include "../AmigoPJT/client.cpp"
#include "../AmigoPJT/inputStream.cpp"
#include "../AmigoPJT/outputStream.cpp"
#include "../AmigoPJT/amigoDatabase.cpp"
#include "../AmigoPJT/amigoSearchEngine.cpp"
#include "../AmigoPJT/util.h"

#include <unordered_map>
#include <iostream>
#include <utility>

vector<Employee> employees;

namespace IntergrationTest
{
    TEST(AmigoIntergtaionTest, Sample_TC_Test)
    {
        string input_path = "..\\AmigoPJT\\input_20_20.txt";
        string refer_path = "..\\AmigoPJT\\output_20_20_ref.txt";
        string output_path = "output_20_20.txt";

        Client amigo_client{ input_path, output_path };
        amigo_client.Run(true);

        EXPECT_EQ(0, Util::Compare(refer_path, output_path));
    }
}

namespace CommandTest
{
    class AmigoCommandTest : public ::testing::Test
    {
    protected:
        static const int MAX_COUNT = 500;
        char RAND_STR_LIST[MAX_COUNT][8] = { 0, };
        char RAND_NUM_LIST[MAX_COUNT][5] = { 0, };
        set<string> used_emp_num;

        void SetUp()
        {
            used_emp_num.clear();
        }

        void make_string()
        {
            srand((unsigned int)time(NULL));
            for (int i = 0; i < MAX_COUNT; i++)
            {
                const unsigned int len = 1 + rand() % 6;
                for (int c = 0; c < len; c++)
                {
                    RAND_STR_LIST[i][c] = 'A' + rand() % 26;
                }
                RAND_STR_LIST[i][len] = NULL;
            }
        }

        void make_number()
        {
            srand((unsigned int)time(NULL));
            for (int i = 0; i < MAX_COUNT; i++)
            {
                for (int c = 0; c < 4; c++)
                {
                    RAND_NUM_LIST[i][c] = '0' + rand() % 10;
                }
                RAND_NUM_LIST[i][5] = NULL;
            }
        }

        string make_date(const int max)
        {
            unsigned int XX = rand() % max + 1;
            if (XX < 10)
            {
                return "0" + to_string(XX);
            }
            return to_string(XX);
        }

        string make_employee_num()
        {
            string result;
            do
            {
                char front[5] = { 0, };
                unsigned int YY = rand() % 100;
                if (YY > 21 && YY < 69)
                {
                    YY = YY % 22;
                }
                sprintf(front, "%02d%02d", YY, rand() % 100);
                result = string(front) + RAND_NUM_LIST[rand() % MAX_COUNT];
            } while (used_emp_num.find(result) != used_emp_num.end());
            used_emp_num.insert(result);
            return result;
        }

        void QueryCommand(const vector<Command>& commands)
        {
            auto amigo_db = new AmigoDatabase();
            for (size_t i = 0; i < commands.size(); i++)
            {
                string result = amigo_db->Query(commands[i]);
                if (result.length() > 1)
                {
                    cout << result << endl;
                }
            }
            delete amigo_db;
        }

        Command GenerateCommand(const vector<string>& raw_command)
        {
            int i = 0;
            Command command;
            for (const string param : raw_command)
            {
                command.param[i++] = param;
            }
            command.is_valid = true;
            return command;
        }
    };

    TEST_F(AmigoCommandTest, Unsupported_Command)
    {
        vector<string> raw_command
        {
            "SWAP","op1","op2","op3","employeeNum", "name", "cl", "phoneNum", "birthday", "certi"
        };

        vector<Command> commands{ GenerateCommand(raw_command) };

        EXPECT_THROW(QueryCommand(commands), runtime_error);
    }

    TEST_F(AmigoCommandTest, Supported_All_Command)
    {
        vector<vector<string>> raw_commands
        {
            { "ADD", " ", " ", " ", "15123099", "VXIHXOTH JHOP", "CL3", "010-3112-2609", "19771211", "ADV" },
            { "ADD", " ", " ", " ", "17112609", "FB NTAWR", "CL4", "010-5645-6122", "19861203", "PRO" },
            { "ADD", " ", " ", " ", "18115040", "TTETHU HBO", "CL3", "010-4581-2050", "20080718", "ADV" },
            { "SCH", "-p", "-d", " ", "birthday", "04" },
            { "MOD", "-p", " ", " ", "name", "FB NTAWR", "birthday", "20050520" },
            { "DEL", " ", " ", " ", "employeeNum", "18115040" }
        };

        vector<Command> commands;
        commands.reserve(raw_commands.size());

        for (const auto& raw_command : raw_commands)
        {
            int i = 0;
            Command command;
            for (const string param : raw_command)
            {
                command.param[i++] = param;
            }
            commands.emplace_back(command);
        }

        EXPECT_NO_THROW(QueryCommand(commands));
    }    

    TEST_F(AmigoCommandTest, DISABLED_Heavy_Command_List)
    {
        const unsigned int add_count = 200000;
        const unsigned int cmd_count = 100000;

        vector<Command> commands;
        commands.reserve(add_count + cmd_count);

        make_string();
        make_number();

        const string certi[] = { "ADV", "PRO", "EX" };
        // DEL 자료가 너무 금방 지워져서 SCH로 대체
        const string cmd_list[] = { "SCH","SCH","MOD" };
        const string col_list[] = { "employeeNum", "name", "cl", "phoneNum", "birthday", "certi" };
        const string op2_list[][4]
        {
            { " ",  " ",  " ",  " "},
            { "-f", "-l", " ",  " "},
            { " ",  " ",  " ",  " "},
            { "-m", "-l", " ",  " "},
            { "-y", "-m", "-d", " "},
            { " ",  " ",  " ",  " "}
        };

        srand((unsigned int)time(NULL));

        auto PickRandomData = [&](const int& col, const int& op2 = 0) {
            switch (col)
            {
            case 0:
                return make_employee_num();
            case 1:
                if (op2 >= 2) // Full name
                {
                    return RAND_STR_LIST[rand() % MAX_COUNT] + string(" ") + RAND_STR_LIST[rand() % MAX_COUNT];
                }
                return RAND_STR_LIST[rand() % MAX_COUNT] + string("");
            case 2:
                return "CL" + to_string(rand() % 4 + 1);
            case 3:
                if (op2 >= 2) // Full number
                {
                    return string("010-") + RAND_NUM_LIST[rand() % MAX_COUNT] + string("-") + RAND_NUM_LIST[rand() % MAX_COUNT];
                }
                return RAND_NUM_LIST[rand() % MAX_COUNT] + string("");
            case 4:
                if (op2 >= 3) // Full date
                {
                    return to_string(1960 + rand() % 100) + make_date(12) + make_date(31);
                }
                return (op2 == 0) ? to_string(1960 + rand() % 100) : (op2 == 1) ? make_date(12) : make_date(31);
            case 5:
                return certi[rand() % 3];
            }
            return string(" ");
        };

        for (int i = 0; i < add_count; i++)
        {
            Command command;
            command.param[0] = "ADD";
            command.param[1] = " ";
            command.param[2] = " ";
            command.param[3] = " ";
            command.param[4] = PickRandomData(0);
            command.param[5] = PickRandomData(1, 2);
            command.param[6] = PickRandomData(2);
            command.param[7] = PickRandomData(3, 2);
            command.param[8] = PickRandomData(4, 3);
            command.param[9] = PickRandomData(5);
            command.is_valid = true;
            commands.emplace_back(command);
        }

        for (int i = 0; i < cmd_count; i++)
        {
            Command command;
            unsigned int cmd = rand() % 3;
            unsigned int op2 = rand() % 3;
            unsigned int col = rand() % 6;
            command.param[0] = cmd_list[cmd];
            command.param[1] = (i % 2) == 0 ? " " : "-p";
            command.param[2] = op2_list[col][op2];
            command.param[3] = " ";
            command.param[4] = col_list[col];
            command.param[5] = PickRandomData(col, op2);
            if (cmd == 2)
            {
                unsigned int op2 = rand() % 3;
                unsigned int col = rand() % 5 + 1;
                command.param[6] = col_list[col];
                command.param[7] = PickRandomData(col, op2);
            }
            command.is_valid = true;
            commands.emplace_back(command);
        }

        EXPECT_NO_THROW(QueryCommand(commands));
    }
}

namespace AddTest 
{
    class AmigoADDTest : public ::testing::Test
    {
    protected:
        Command GenerateCommand(const vector<string>& raw_command)
        {
            int i = 0;
            Command command;
            for (const string param : raw_command)
            {
                command.param[i++] = param;
            }
            command.is_valid = true;
            return command;
        }

        AmigoDatabase amigo_db;

        vector<vector<string>> raw_commands
        {
            { "ADD", " ", " ", " ", "17112609", "FB NTAWR",       "CL4", "010-5645-6122", "19861203", "PRO" },
            { "ADD", " ", " ", " ", "02117175", "SBILHUT LDEXRI", "CL4", "010-2814-1699", "19950704", "ADV" },
            { "ADD", " ", " ", " ", "08123556", "WN XV",          "CL1", "010-7986-5047", "20100614", "PRO" },
            { "ADD", " ", " ", " ", "85125741", "FBAH RTIJ",      "CL1", "010-8900-1478", "19780228", "ADV" },
            { "ADD", " ", " ", " ", "11109136", "QKAHCEX LTODDO", "CL4", "010-2627-8566", "19640130", "PRO" },
            { "ADD", " ", " ", " ", "08108827", "RTAH VNUP",      "CL4", "010-9031-2726", "19780417", "ADV" },
            { "ADD", " ", " ", " ", "05101762", "VCUHLE HMU",     "CL4", "010-3988-9289", "20030819", "PRO" }
        };
    };

    TEST_F(AmigoADDTest, ADD_Unique_Key_Data)
    {
        for (const auto& raw_command : raw_commands)
        {
            EXPECT_NO_THROW(amigo_db.Query(GenerateCommand(raw_command)));
        }
    }

    TEST_F(AmigoADDTest, ADD_Duplicate_Key_Data)
    {
        for (const auto& raw_command : raw_commands)
        {
            EXPECT_NO_THROW(amigo_db.Query(GenerateCommand(raw_command)));
        }

        vector<string> duplicate
        {
            "ADD", " ", " ", " ", "02117175", "SBILHUT HMU", "CL4", "010-2814-2627", "19750904", "ADV"
        };
        EXPECT_THROW(amigo_db.Query(GenerateCommand(duplicate)), invalid_argument);
    }
}

namespace DelTest
{
    vector<vector<string>> testData =
    {
        { "ADD", " ", " ", " ", "15123099", "VXIHXOTH JHOP" , "CL3", "010-3112-2609", "19771211", "ADV"},
        { "ADD", " ", " ", " ", "17112609", "FB NTAWR"      , "CL4", "010-5645-6122", "19861203", "PRO"},
        { "ADD", " ", " ", " ", "18115040", "TTETHU HBO"    , "CL3", "010-4581-2050", "20080718", "ADV"},
        { "ADD", " ", " ", " ", "88114052", "NQ LVARW"      , "CL4", "010-4528-3059", "19911021", "PRO"},
        { "ADD", " ", " ", " ", "19129568", "SRERLALH HMEF" , "CL2", "010-3091-9521", "19640910", "PRO"},
        { "ADD", " ", " ", " ", "17111236", "VSID TVO"      , "CL1", "010-3669-1077", "20120718", "PRO"},
        { "ADD", " ", " ", " ", "18117906", "TWU QSOLT"     , "CL4", "010-6672-7186", "20030413", "EX"},
        { "ADD", " ", " ", " ", "08123556", "WN XV"         , "CL1", "010-7986-5047", "20100614", "PRO"}
    };

    Command GenerateCommand(const vector<string>& raw_command)
    {
        int i = 0;
        Command command;
        for (const string param : raw_command)
        {
            command.param[i++] = param;
        }
        command.is_valid = true;
        return command;
    }

    TEST(AmigoDelTest, DelCLTest)
    {
        AmigoDatabase newdata1, newdata2, newdata3, newdata4;
        for (const auto& rawData : testData)
        {
            newdata1.Query(GenerateCommand(rawData));
            newdata2.Query(GenerateCommand(rawData));
            newdata3.Query(GenerateCommand(rawData));
            newdata4.Query(GenerateCommand(rawData));
        }
        Command command1 = GenerateCommand({ "DEL", " ", " ", " ", "cl", "CL4" });
        Command command2 = GenerateCommand({ "DEL", " ", " ", " ", "cl", "CL3" });
        Command command3 = GenerateCommand({ "DEL", " ", " ", " ", "cl", "CL2" });
        Command command4 = GenerateCommand({ "DEL", " ", " ", " ", "cl", "CL1" });

        EXPECT_EQ("DEL,3", newdata1.Query(command1));
        EXPECT_EQ("DEL,2", newdata2.Query(command2));
        EXPECT_EQ("DEL,1", newdata3.Query(command3));
        EXPECT_EQ("DEL,2", newdata4.Query(command4));
    }

    TEST(AmigoDelTest, DelNameTest)
    {
        AmigoDatabase newdata1, newdata2;
        for (const auto& rawData : testData)
        {
            newdata1.Query(GenerateCommand(rawData));
            newdata2.Query(GenerateCommand(rawData));
        }
        Command command1 = GenerateCommand({ "DEL", " ", " ", " ", "name", "TWU QSOLT" });
        Command command2 = GenerateCommand({ "DEL", " ", " ", " ", "name", "ASDF QWER" });

        EXPECT_EQ("DEL,1", newdata1.Query(command1));
        EXPECT_EQ("DEL,NONE", newdata2.Query(command2));
    }

    TEST(AmigoDelTest, DelEmployNumTest)
    {
        AmigoDatabase newdata1, newdata2;
        for (const auto& rawData : testData)
        {
            newdata1.Query(GenerateCommand(rawData));
            newdata2.Query(GenerateCommand(rawData));
        }
        Command command1 = GenerateCommand({ "DEL", " ", " ", " ", "employeeNum", "17112609" });
        Command command2 = GenerateCommand({ "DEL", " ", " ", " ", "employeeNum", "12312312" });

        EXPECT_EQ("DEL,1", newdata1.Query(command1));
        EXPECT_EQ("DEL,NONE", newdata2.Query(command2));
    }

    TEST(AmigoDelTest, DelPhoneNumTest)
    {
        AmigoDatabase newdata1, newdata2;
        for (const auto& rawData : testData)
        {
            newdata1.Query(GenerateCommand(rawData));
            newdata2.Query(GenerateCommand(rawData));
        }
        Command command1 = GenerateCommand({ "DEL", " ", " ", " ", "phoneNum", "010-4528-3059" });
        Command command2 = GenerateCommand({ "DEL", " ", " ", " ", "phoneNum", "010-1111-0000" });

        EXPECT_EQ("DEL,1", newdata1.Query(command1));
        EXPECT_EQ("DEL,NONE", newdata2.Query(command2));
    }

    TEST(AmigoDelTest, DelBirthdayTest)
    {
        AmigoDatabase newdata1, newdata2;
        for (const auto& rawData : testData)
        {
            newdata1.Query(GenerateCommand(rawData));
            newdata2.Query(GenerateCommand(rawData));
        }
        Command command1 = GenerateCommand({ "DEL", " ", " ", " ", "birthday", "19640910" });
        Command command2 = GenerateCommand({ "DEL", " ", " ", " ", "birthday", "20101010" });

        EXPECT_EQ("DEL,1", newdata1.Query(command1));
        EXPECT_EQ("DEL,NONE", newdata2.Query(command2));
    }

    TEST(AmigoDelTest, DelCertiTest)
    {
        AmigoDatabase newdata1, newdata2, newdata3;
        for (const auto& rawData : testData)
        {
            newdata1.Query(GenerateCommand(rawData));
            newdata2.Query(GenerateCommand(rawData));
            newdata3.Query(GenerateCommand(rawData));
        }
        Command command1 = GenerateCommand({ "DEL", " ", " ", " ", "certi", "ADV" });
        Command command2 = GenerateCommand({ "DEL", " ", " ", " ", "certi", "PRO" });
        Command command3 = GenerateCommand({ "DEL", " ", " ", " ", "certi", "EX" });

        EXPECT_EQ("DEL,2", newdata1.Query(command1));
        EXPECT_EQ("DEL,5", newdata2.Query(command2));
        EXPECT_EQ("DEL,1", newdata3.Query(command3));
    }
}

namespace ModTest
{
    class AmigoModTest : public AddTest::AmigoADDTest
    {
    protected:
        void SetUp()
        {
            for (const auto& raw_command : raw_commands)
            {
                amigo_db.Query(GenerateCommand(raw_command));
            }
        }
    };

    TEST_F(AmigoModTest, Found_0_Record_0_Modify_Nothing)
    {
        Command command = GenerateCommand({ "MOD", "-p", " ", " ", "name", "NO ONE", "birthday", "20050520" });

        string result = amigo_db.Query(command);

        EXPECT_STREQ("MOD,NONE", result.c_str());
    }

    TEST_F(AmigoModTest, Found_1_Throw_Exception_Unknown_Column)
    {
        Command command = GenerateCommand({ "MOD", "-p", " ", " ", "name", "FB NTAWR", "holiday", "20050520" });

        EXPECT_THROW(amigo_db.Query(command), invalid_argument);
    }

    TEST_F(AmigoModTest, Found_1_Throw_Exception_Modify_EmployeeNum)
    {
        Command command = GenerateCommand({ "MOD", "-p", " ", " ", "name", "FB NTAWR", "employeeNum", "88114052" });

        EXPECT_THROW(amigo_db.Query(command), invalid_argument);
    }

    TEST_F(AmigoModTest, Found_1_Record_1_Modify_Birthday)
    {
        Command command = GenerateCommand({ "MOD", "-p", " ", " ", "name", "FB NTAWR", "birthday", "20050520" });

        const string expect_result = "MOD,17112609,FB NTAWR,CL4,010-5645-6122,19861203,PRO";

        const string actual_result = amigo_db.Query(command);

        EXPECT_STREQ(expect_result.c_str(), actual_result.c_str());
    }

    TEST_F(AmigoModTest, Found_1_Record_1_Modify_Birthday_Updated)
    {
        Command command = GenerateCommand({ "MOD", "-p", " ", " ", "name", "FB NTAWR", "birthday", "20050520" });

        amigo_db.Query(command);

        const string expect_result = "MOD,17112609,FB NTAWR,CL4,010-5645-6122,20050520,PRO";

        const string actual_result = amigo_db.Query(command);

        EXPECT_STREQ(expect_result.c_str(), actual_result.c_str());
    }

    TEST_F(AmigoModTest, Found_5_Record_5_Modify_Name_Updated)
    {
        Command command = GenerateCommand({ "MOD", "-p", " ", " ", "cl", "CL4", "name", "Anonymous" });

        amigo_db.Query(command);

        const string expect_result
        {
            "MOD,02117175,Anonymous,CL4,010-2814-1699,19950704,ADV\n"\
            "MOD,05101762,Anonymous,CL4,010-3988-9289,20030819,PRO\n"\
            "MOD,08108827,Anonymous,CL4,010-9031-2726,19780417,ADV\n"\
            "MOD,11109136,Anonymous,CL4,010-2627-8566,19640130,PRO\n"\
            "MOD,17112609,Anonymous,CL4,010-5645-6122,19861203,PRO"
        };

        const string actual_result = amigo_db.Query(command);

        EXPECT_STREQ(expect_result.c_str(), actual_result.c_str());
    }

    TEST_F(AmigoModTest, Found_5_Record_5_Modify_CL_Updated)
    {
        Command command = GenerateCommand({ "MOD", "-p", " ", " ", "cl", "CL4", "cl", "CL3" });

        amigo_db.Query(command);

        const string actual_result = amigo_db.Query(command);

        EXPECT_STREQ("MOD,NONE", actual_result.c_str());
    }

    TEST_F(AmigoModTest, Found_5_Record_5_Modify_PhoneNum_Updated)
    {
        Command command = GenerateCommand({ "MOD", "-p", " ", " ", "cl", "CL4", "phoneNum", "010-1234-0000" });

        amigo_db.Query(command);

        const string expect_result
        {
            "MOD,02117175,SBILHUT LDEXRI,CL4,010-1234-0000,19950704,ADV\n"\
            "MOD,05101762,VCUHLE HMU,CL4,010-1234-0000,20030819,PRO\n"\
            "MOD,08108827,RTAH VNUP,CL4,010-1234-0000,19780417,ADV\n"\
            "MOD,11109136,QKAHCEX LTODDO,CL4,010-1234-0000,19640130,PRO\n"\
            "MOD,17112609,FB NTAWR,CL4,010-1234-0000,19861203,PRO"
        };

        const string actual_result = amigo_db.Query(command);

        EXPECT_STREQ(expect_result.c_str(), actual_result.c_str());
    }

    TEST_F(AmigoModTest, Found_5_Record_5_Modify_Birthday_Updated)
    {
        Command command = GenerateCommand({ "MOD", "-p", " ", " ", "cl", "CL4", "birthday", "20050520" });

        amigo_db.Query(command);

        const string expect_result
        {
            "MOD,02117175,SBILHUT LDEXRI,CL4,010-2814-1699,20050520,ADV\n"\
            "MOD,05101762,VCUHLE HMU,CL4,010-3988-9289,20050520,PRO\n"\
            "MOD,08108827,RTAH VNUP,CL4,010-9031-2726,20050520,ADV\n"\
            "MOD,11109136,QKAHCEX LTODDO,CL4,010-2627-8566,20050520,PRO\n"\
            "MOD,17112609,FB NTAWR,CL4,010-5645-6122,20050520,PRO"
        };

        const string actual_result = amigo_db.Query(command);

        EXPECT_STREQ(expect_result.c_str(), actual_result.c_str());
    }

    TEST_F(AmigoModTest, Found_5_Record_5_Modify_Certi_Updated)
    {
        Command command = GenerateCommand({ "MOD", "-p", " ", " ", "cl", "CL4", "certi", "EX" });

        amigo_db.Query(command);

        const string expect_result
        {
            "MOD,02117175,SBILHUT LDEXRI,CL4,010-2814-1699,19950704,EX\n"\
            "MOD,05101762,VCUHLE HMU,CL4,010-3988-9289,20030819,EX\n"\
            "MOD,08108827,RTAH VNUP,CL4,010-9031-2726,19780417,EX\n"\
            "MOD,11109136,QKAHCEX LTODDO,CL4,010-2627-8566,19640130,EX\n"\
            "MOD,17112609,FB NTAWR,CL4,010-5645-6122,19861203,EX"
        };

        const string actual_result = amigo_db.Query(command);

        EXPECT_STREQ(expect_result.c_str(), actual_result.c_str());
    }
}

namespace SeachTest
{
    class AmigoSchTest : public AddTest::AmigoADDTest
    {
    protected:
        void SetUp()
        {
            vector<vector<string>> raw_commands
            {
                { "ADD", " ", " ", " ", "85125741", "FBAH RTIJ",      "CL1", "010-8900-1478", "19780228", "ADV" },
                { "ADD", " ", " ", " ", "11109136", "QKAHCEX LTODDO", "CL4", "010-2627-8566", "19640130", "PRO" },
                { "ADD", " ", " ", " ", "08108827", "RTAH VNUP",      "CL4", "010-9031-2726", "19780417", "ADV" },
                { "ADD", " ", " ", " ", "05101762", "VCUHLE HMU",     "CL4", "010-3988-9289", "20030819", "PRO" },
                { "ADD", " ", " ", " ", "15123099", "VXIHXOTH JHOP" , "CL3", "010-3112-2609", "19771211", "ADV" },
                { "ADD", " ", " ", " ", "17112609", "FB NTAWR"      , "CL4", "010-5645-6122", "19861203", "PRO" },
                { "ADD", " ", " ", " ", "18115040", "TTETHU HBO"    , "CL3", "010-4581-2050", "20080718", "ADV" },
                { "ADD", " ", " ", " ", "88114052", "NQ LVARW"      , "CL4", "010-4528-3059", "19911021", "PRO" },
                { "ADD", " ", " ", " ", "19129568", "SRERLALH HMEF" , "CL2", "010-3091-9521", "19640910", "PRO" },
                { "ADD", " ", " ", " ", "17111236", "VSID TVO"      , "CL1", "010-3669-1077", "20120718", "PRO" },
                { "ADD", " ", " ", " ", "18117906", "TWU QSOLT"     , "CL4", "010-6672-7186", "20030413", "PRO" },
                { "ADD", " ", " ", " ", "08123556", "WN XV"         , "CL3", "010-7986-5047", "20100614", "PRO" },
                { "ADD", " ", " ", " ", "02117175", "SBILHUT LDEXRI", "CL4", "010-2814-1699", "19950704", "ADV" },
                { "ADD", " ", " ", " ", "03113260", "HH LTUPF"      , "CL2", "010-5798-5383", "19781018", "PRO" },
                { "ADD", " ", " ", " ", "14130827", "RPO JK"        , "CL4", "010-4528-1698", "20090201", "ADV" },
                { "ADD", " ", " ", " ", "01122329", "TWU WD"        , "CL4", "010-7174-5680", "20071117", "PRO" }
            };

            for (const auto& raw_command : raw_commands)
            {
                amigo_db.Query(GenerateCommand(raw_command));
            }
        }
    };

    TEST_F(AmigoSchTest, TestByFullBirth_Pass)
    {
        Command command = GenerateCommand({ "SCH", "-p", " ", " ", "birthday", "19780228", " ", " " });

        amigo_db.Query(command);

        const string expect_result
        {
            "SCH,85125741,FBAH RTIJ,CL1,010-8900-1478,19780228,ADV"
        };

        const string actual_result = amigo_db.Query(command);

        EXPECT_STREQ(expect_result.c_str(), actual_result.c_str());
    }
    TEST_F(AmigoSchTest, TestByFullBirth_Fail)
    {
        Command command = GenerateCommand({ "SCH", "-p", " ", " ", "birthday", "19780227", " ", " " });

        amigo_db.Query(command);

        const string expect_result
        {
            "SCH,85125741,FBAH RTIJ,CL1,010-8900-1478,19780228,ADV"
        };

        const string actual_result = amigo_db.Query(command);

        EXPECT_STRNE(expect_result.c_str(), actual_result.c_str());
    }
    TEST_F(AmigoSchTest, TestByFullBirthYear_Pass)
    {
        Command command = GenerateCommand({ "SCH", "-p", "-y", " ", "birthday", "1978", " ", " " });

        amigo_db.Query(command);

        const string expect_result
        {
            "SCH,85125741,FBAH RTIJ,CL1,010-8900-1478,19780228,ADV\n"\
            "SCH,03113260,HH LTUPF,CL2,010-5798-5383,19781018,PRO\n"\
            "SCH,08108827,RTAH VNUP,CL4,010-9031-2726,19780417,ADV"
        };

        const string actual_result = amigo_db.Query(command);

        EXPECT_STREQ(expect_result.c_str(), actual_result.c_str());
    }
    TEST_F(AmigoSchTest, TestByFullBirthYear_Fail)
    {
        Command command = GenerateCommand({ "SCH", "-p", "-y", " ", "birthday", "2003", " ", " " });

        amigo_db.Query(command);

        const string expect_result
        {
            "SCH,85125741,FBAH RTIJ,CL1,010-8900-1478,19780228,ADV\n"\
            "SCH,08108827,RTAH VNUP,CL4,010-9031-2726,19780417,ADV"
        };

        const string actual_result = amigo_db.Query(command);

        EXPECT_STRNE(expect_result.c_str(), actual_result.c_str());
    }
    TEST_F(AmigoSchTest, TestByFullBirthYear_NONE)
    {
        Command command = GenerateCommand({ "SCH", " ", "-y", " ", "birthday", "2000", " ", " " });

        amigo_db.Query(command);

        const string expect_result
        {
            "SCH,NONE"
        };

        const string actual_result = amigo_db.Query(command);

        EXPECT_STREQ(expect_result.c_str(), actual_result.c_str());
    }
    TEST_F(AmigoSchTest, TestByFullBirthMonth_Pass)
    {
        Command command = GenerateCommand({ "SCH", "-p", "-m", " ", "birthday", "12", " ", " " });

        amigo_db.Query(command);

        const string expect_result
        {
            "SCH,15123099,VXIHXOTH JHOP,CL3,010-3112-2609,19771211,ADV\n"\
            "SCH,17112609,FB NTAWR,CL4,010-5645-6122,19861203,PRO"
        };

        const string actual_result = amigo_db.Query(command);

        EXPECT_STREQ(expect_result.c_str(), actual_result.c_str());
    }
    TEST_F(AmigoSchTest, TestByFullBirthMonth_Fail)
    {
        Command command = GenerateCommand({ "SCH", "-p", "-m", " ", "birthday", "06", " ", " " });

        amigo_db.Query(command);

        const string expect_result
        {
            "SCH,17112609,FB NTAWR,CL4,010-5645-6122,19861203,PRO"
        };

        const string actual_result = amigo_db.Query(command);

        EXPECT_STRNE(expect_result.c_str(), actual_result.c_str());
    }
    TEST_F(AmigoSchTest, TestByFullBirthDate_Pass)
    {
        Command command = GenerateCommand({ "SCH", "-p", "-d", " ", "birthday", "03", " ", " " });

        amigo_db.Query(command);

        const string expect_result
        {
            "SCH,17112609,FB NTAWR,CL4,010-5645-6122,19861203,PRO"
        };

        const string actual_result = amigo_db.Query(command);

        EXPECT_STREQ(expect_result.c_str(), actual_result.c_str());
    }
    TEST_F(AmigoSchTest, TestByFullBirthDate_Fail)
    {
        Command command = GenerateCommand({ "SCH", "-p", "-d", " ", "birthday", "06", " ", " " });

        amigo_db.Query(command);

        const string expect_result
        {
            "SCH,17112609,FB NTAWR,CL4,010-5645-6122,19861203,PRO"
        };

        const string actual_result = amigo_db.Query(command);

        EXPECT_STRNE(expect_result.c_str(), actual_result.c_str());
    }
    TEST_F(AmigoSchTest, SearchBirthdayException) {
        Command command = GenerateCommand({ "SCH", "-p", " ", " ", "birthday", "19871320", " ", " " });
        EXPECT_ANY_THROW(amigo_db.Query(command));
        command = GenerateCommand({ "SCH", "-p", " ", " ", "birthday", "19871232", " ", " " });
        EXPECT_ANY_THROW(amigo_db.Query(command));
        command = GenerateCommand({ "SCH", "-p", " ", " ", "birthday", "871112", " ", " " });
        EXPECT_ANY_THROW(amigo_db.Query(command));
        command = GenerateCommand({ "SCH", "-p", "-y", " ", "birthday", "89", " ", " " });
        EXPECT_ANY_THROW(amigo_db.Query(command));
        command = GenerateCommand({ "SCH", "-p", "-y", " ", "birthday", "19302", " ", " " });
        EXPECT_ANY_THROW(amigo_db.Query(command));
        command = GenerateCommand({ "SCH", "-p", "-m", " ", "birthday", "2", " ", " " });
        EXPECT_ANY_THROW(amigo_db.Query(command));
        command = GenerateCommand({ "SCH", "-p", "-m", " ", "birthday", "14", " ", " " });
        EXPECT_ANY_THROW(amigo_db.Query(command));
        command = GenerateCommand({ "SCH", "-p", "-m", " ", "birthday", "234", " ", " " });
        EXPECT_ANY_THROW(amigo_db.Query(command));
        command = GenerateCommand({ "SCH", "-p", "-d", " ", "birthday", "2", " ", " " });
        EXPECT_ANY_THROW(amigo_db.Query(command));
        command = GenerateCommand({ "SCH", "-p", "-d", " ", "birthday", "32", " ", " " });
        EXPECT_ANY_THROW(amigo_db.Query(command));
        command = GenerateCommand({ "SCH", "-p", "-d", " ", "birthday", "245", " ", " " });
        EXPECT_ANY_THROW(amigo_db.Query(command));
        command = GenerateCommand({ "SCH", "-p", "-m", " ", "birthday", "11", " ", " " });
        EXPECT_NO_THROW(amigo_db.Query(command));
        command = GenerateCommand({ "SCH", "-p", " ", " ", "birthday", "19991231", " ", " " });
        EXPECT_NO_THROW(amigo_db.Query(command));
        command = GenerateCommand({ "SCH", "-p", "-y", " ", "birthday", "1999", " ", " " });
        EXPECT_NO_THROW(amigo_db.Query(command));
        command = GenerateCommand({ "SCH", "-p", "-m", " ", "birthday", "11", " ", " " });
        EXPECT_NO_THROW(amigo_db.Query(command));
        command = GenerateCommand({ "SCH", "-p", "-d", " ", "birthday", "11", " ", " " });
        EXPECT_NO_THROW(amigo_db.Query(command));
        command = GenerateCommand({ "SCH", "-p", " ", " ", "birthday", "YYYYMMDD", " ", " " });
        EXPECT_ANY_THROW(amigo_db.Query(command));

    }

    TEST_F(AmigoSchTest, SearchFullName)
    {
        Command command = GenerateCommand({ "SCH", "-p", " ", " ", "name", "SRERLALH HMEF", " ", " "});

        amigo_db.Query(command);

        const string expect_result
        {
            "SCH,19129568,SRERLALH HMEF,CL2,010-3091-9521,19640910,PRO"
        };

        const string actual_result = amigo_db.Query(command);

        EXPECT_EQ(expect_result, actual_result);
    }

    TEST_F(AmigoSchTest, SearchFirstName) {
        Command command = GenerateCommand({ "SCH", "-p", "-f", " ", "name", "RPO", " ", " " });

        amigo_db.Query(command);

        const string expect_result
        {
             "SCH,14130827,RPO JK,CL4,010-4528-1698,20090201,ADV"
        };

        const string actual_result = amigo_db.Query(command);

        EXPECT_EQ(expect_result, actual_result);
    }

    TEST_F(AmigoSchTest, SearchLastName) {
        Command command = GenerateCommand({ "SCH", "-p", "-l", " ", "name", "WD", " ", " " });

        amigo_db.Query(command);

        const string expect_result
        {
             "SCH,01122329,TWU WD,CL4,010-7174-5680,20071117,PRO"
        };

        const string actual_result = amigo_db.Query(command);

        EXPECT_EQ(expect_result, actual_result);
    }

    TEST_F(AmigoSchTest, SearchNameNone) {
        Command command = GenerateCommand({ "SCH", "-p", "-l", " ", "name", "WNGD", " ", " " });

        amigo_db.Query(command);

        const string expect_result
        {
             "SCH,NONE"
        };

        const string actual_result = amigo_db.Query(command);

        EXPECT_EQ(expect_result, actual_result);
    }

    TEST_F(AmigoSchTest, SearchNameException) {
        Command command = GenerateCommand({ "SCH", "-p", " ", " ", "name", "FWEFWEFWNGD", " ", " " });
        EXPECT_ANY_THROW(amigo_db.Query(command));
        command = GenerateCommand({ "SCH", "-p", " ", " ", "name", "FWBBBBBBBBBB EFWEFWNGD", " ", " " });
        EXPECT_ANY_THROW(amigo_db.Query(command));
        command = GenerateCommand({ "SCH", "-p", " ", " ", "name", "fsjlfj WFWFAWF", " ", " " });
        EXPECT_ANY_THROW(amigo_db.Query(command));
        command = GenerateCommand({ "SCH", "-p", " ", " ", "name", "FWBB EFWD", " ", " " });
        EXPECT_NO_THROW(amigo_db.Query(command));
        command = GenerateCommand({ "SCH", "-p", " ", " ", "name", "FWBBB EFWEF", " ", " " });
        EXPECT_NO_THROW(amigo_db.Query(command));
        command = GenerateCommand({ "SCH", "-p", " ", " ", "name", "FWBBBB EFWEFW", " ", " " });
        EXPECT_NO_THROW(amigo_db.Query(command));
        command = GenerateCommand({ "SCH", "-p", " ", " ", "name", "FWBBBBB EFWEFWN", " ", " " });
        EXPECT_NO_THROW(amigo_db.Query(command));
        command = GenerateCommand({ "SCH", "-p", " ", " ", "name", "FWBBBBBD EFWEFWNGD", " ", " " });
        EXPECT_ANY_THROW(amigo_db.Query(command));

    }

    TEST_F(AmigoSchTest, SearchFullPhoneNumber)
    {
        Command command = GenerateCommand({ "SCH", "-p", " ", " ", "phoneNum", "010-3669-1077", " ", " " });

        const string expect_result
        {
            "SCH,17111236,VSID TVO,CL1,010-3669-1077,20120718,PRO"
        };

        const string actual_result = amigo_db.Query(command);

        EXPECT_EQ(expect_result, actual_result);
    }

    TEST_F(AmigoSchTest, SearchMiddlePhoneNumber) {
        Command command = GenerateCommand({ "SCH", "-p", "-m", " ", "phoneNum", "7174", " ", " " });

        const string expect_result
        {
             "SCH,01122329,TWU WD,CL4,010-7174-5680,20071117,PRO"
        };

        const string actual_result = amigo_db.Query(command);

        EXPECT_EQ(expect_result, actual_result);
    }

    TEST_F(AmigoSchTest, SearchLastPhoneNumber) {
        Command command = GenerateCommand({ "SCH", "-p", "-l", " ", "phoneNum", "1698", " ", " " });

        const string expect_result
        {
             "SCH,14130827,RPO JK,CL4,010-4528-1698,20090201,ADV"
        };

        const string actual_result = amigo_db.Query(command);

        EXPECT_EQ(expect_result, actual_result);
    }

    TEST_F(AmigoSchTest, SearchPhoneNumberNone) {
        Command command = GenerateCommand({ "SCH", "-p", " ", " ", "phoneNum", "010-9999-1111", " ", " " });

        const string expect_result
        {
             "SCH,NONE"
        };

        const string actual_result = amigo_db.Query(command);

        EXPECT_EQ(expect_result, actual_result);
    }

    TEST_F(AmigoSchTest, SearchPhoneNumberException) {
        Command command = GenerateCommand({ "SCH", "-p", " ", " ", "phoneNum", "016-9999-1111", " ", " " });
        EXPECT_ANY_THROW(amigo_db.Query(command));
        command = GenerateCommand({ "SCH", "-p", " ", " ", "phoneNum", "010-999-1111", " ", " " });
        EXPECT_ANY_THROW(amigo_db.Query(command));
        command = GenerateCommand({ "SCH", "-p", " ", " ", "phoneNum", "01099991111", " ", " " });
        EXPECT_ANY_THROW(amigo_db.Query(command));
        command = GenerateCommand({ "SCH", "-p", " ", " ", "phoneNum", "PHO-NENU-MBER", " ", " " });
        EXPECT_ANY_THROW(amigo_db.Query(command));
        command = GenerateCommand({ "SCH", "-p", " ", " ", "phoneNum", "010-0000-0000", " ", " " });
        EXPECT_NO_THROW(amigo_db.Query(command));
        command = GenerateCommand({ "SCH", "-p", " ", " ", "phoneNum", "010-1111-1111", " ", " " });
        EXPECT_NO_THROW(amigo_db.Query(command));
        command = GenerateCommand({ "SCH", "-p", " ", " ", "phoneNum", "010-2222-2222", " ", " " });
        EXPECT_NO_THROW(amigo_db.Query(command));
        command = GenerateCommand({ "SCH", "-p", " ", " ", "phoneNum", "010-3333-3333", " ", " " });
        EXPECT_NO_THROW(amigo_db.Query(command));
        command = GenerateCommand({ "SCH", "-p", " ", " ", "phoneNum", "010-4444-4444", " ", " " });
        EXPECT_NO_THROW(amigo_db.Query(command));
        command = GenerateCommand({ "SCH", "-p", " ", " ", "phoneNum", "010-5555-5555", " ", " " });
        EXPECT_NO_THROW(amigo_db.Query(command));
        command = GenerateCommand({ "SCH", "-p", " ", " ", "phoneNum", "010-6666-6666", " ", " " });
        EXPECT_NO_THROW(amigo_db.Query(command));
        command = GenerateCommand({ "SCH", "-p", " ", " ", "phoneNum", "010-7777-7777", " ", " " });
        EXPECT_NO_THROW(amigo_db.Query(command));
        command = GenerateCommand({ "SCH", "-p", " ", " ", "phoneNum", "010-8888-8888", " ", " " });
        EXPECT_NO_THROW(amigo_db.Query(command));
        command = GenerateCommand({ "SCH", "-p", " ", " ", "phoneNum", "010-9999-9999", " ", " " });
        EXPECT_NO_THROW(amigo_db.Query(command));
        command = GenerateCommand({ "SCH", "-p", " ", " ", "phoneNum", "010----------", " ", " " });
        EXPECT_ANY_THROW(amigo_db.Query(command));

    }
    
    TEST_F(AmigoSchTest, SearchCerti) {
        Command command = GenerateCommand({ "SCH", "-p", " ", " ", "certi", "ADV", " ", " " });

        const string expect_result
        {
                "SCH,85125741,FBAH RTIJ,CL1,010-8900-1478,19780228,ADV\n"\
                "SCH,02117175,SBILHUT LDEXRI,CL4,010-2814-1699,19950704,ADV\n"\
                "SCH,08108827,RTAH VNUP,CL4,010-9031-2726,19780417,ADV\n"\
                "SCH,14130827,RPO JK,CL4,010-4528-1698,20090201,ADV\n"\
                "SCH,15123099,VXIHXOTH JHOP,CL3,010-3112-2609,19771211,ADV"
        };

        const string actual_result = amigo_db.Query(command);

        EXPECT_EQ(expect_result, actual_result);
    }

    TEST_F(AmigoSchTest, SearchCertiException) {
        Command command = GenerateCommand({ "SCH", "-p", " ", " ", "certi", "IM", " ", " " });
        EXPECT_ANY_THROW(amigo_db.Query(command));
    }

    TEST_F(AmigoSchTest, SearchCl) {
        Command command = GenerateCommand({ "SCH", "-p", " ", " ", "cl", "CL2", " ", " " });
      
        const string expect_result
        {
                "SCH,03113260,HH LTUPF,CL2,010-5798-5383,19781018,PRO\n"\
                "SCH,19129568,SRERLALH HMEF,CL2,010-3091-9521,19640910,PRO"
        };

        const string actual_result = amigo_db.Query(command);

        EXPECT_EQ(expect_result, actual_result);
    }

    TEST_F(AmigoSchTest, SearchClException) {
        Command command = GenerateCommand({ "SCH", "-p", " ", " ", "cl", "CL5", " ", " " });
        EXPECT_ANY_THROW(amigo_db.Query(command));
    }

    TEST_F(AmigoSchTest, SearchEmployeeNumber) {
        Command command = GenerateCommand({ "SCH", "-p", " ", " ", "employeeNum", "18117906", " ", " " });

        const string expect_result
        {
                "SCH,18117906,TWU QSOLT,CL4,010-6672-7186,20030413,PRO"
        };

        const string actual_result = amigo_db.Query(command);

        EXPECT_EQ(expect_result, actual_result);
    }

    TEST_F(AmigoSchTest, SearchemployeeNumException) {
        Command command = GenerateCommand({ "SCH", "-p", " ", " ", "employeeNum", "23110043", " ", " " });
        EXPECT_ANY_THROW(amigo_db.Query(command));
        command = GenerateCommand({ "SCH", "-p", " ", " ", "employeeNum", "50110043", " ", " " });
        EXPECT_ANY_THROW(amigo_db.Query(command));
        command = GenerateCommand({ "SCH", "-p", " ", " ", "employeeNum", "2043", " ", " " });
        EXPECT_ANY_THROW(amigo_db.Query(command));
    }
}
