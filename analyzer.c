#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define MAX_JSON_SIZE 50000

// 문자열 안에서 키워드가 몇 번 등장하는지 세기
int count_keyword(const char *text, const char *keyword) {
    int count = 0;
    const char *p = text;
    while ((p = strstr(p, keyword)) != NULL) {
        count++;
        p += strlen(keyword);
    }
    return count;
}

// 특정 키워드 다음에 나오는 문자열 값 추출 
bool extract_next_string(const char *start, const char *key, char *output) {
    const char *p = strstr(start, key);
    if (!p) return false;
    p = strchr(p, '\"'); 
    if (!p) return false;
    p = strchr(p + 1, '\"'); 
    if (!p) return false;
    const char *begin = p + 1;
    p = strchr(begin, '\"');
    if (!p) return false;
    int len = p - begin;
    strncpy(output, begin, len);
    output[len] = '\0';
    return true;
}

// 함수 정보를 추출하는 함수
void analyze_ast(const char *json) {
    int func_count = 0;
    int if_count = 0;
    const char *p = json;

    printf("🔍 함수 분석 시작\n");

    while ((p = strstr(p, "\"_nodetype\": \"FuncDef\"")) != NULL) {
        func_count++;
        printf("\n🔹 [%d번째 함수]\n", func_count);

        // 함수 이름 추출
        char name[100] = "(unknown)";
        extract_next_string(p, "\"name\"", name);
        printf("  🧩 함수 이름: %s\n", name);

        // 리턴 타입 추출
        char *rtype = strstr(p, "\"names\"");
        if (rtype) {
            char *rbeg = strchr(rtype, '[');
            char *rend = strchr(rtype, ']');
            if (rbeg && rend && rend > rbeg) {
                char rtype_str[50] = {0};
                strncpy(rtype_str, rbeg + 1, rend - rbeg - 1);
                printf("  🔙 리턴 타입: %s\n", rtype_str);
            }
        }

        // 파라미터 추출
        printf("  🛠️ 파라미터 목록:\n");
        const char *arg_ptr = p;
        while ((arg_ptr = strstr(arg_ptr, "\"_nodetype\": \"Decl\"")) != NULL) {
            if (arg_ptr > strstr(p, "\"body\"")) break; 
            char param_name[100] = "(unnamed)";
            extract_next_string(arg_ptr, "\"name\"", param_name);

            const char *ptype = strstr(arg_ptr, "\"names\"");
            char ptype_str[100] = "";
            if (ptype) {
                char *pbeg = strchr(ptype, '[');
                char *pend = strchr(ptype, ']');
                if (pbeg && pend && pend > pbeg) {
                    strncpy(ptype_str, pbeg + 1, pend - pbeg - 1);
                    ptype_str[pend - pbeg - 1] = '\0';
                }
            }

            printf("    - %s: %s\n", param_name, ptype_str);
            arg_ptr += strlen("\"_nodetype\": \"Decl\"");
        }

        // 함수 내부의 if 개수 추출
        int local_if_count = count_keyword(p, "\"_nodetype\": \"If\"");
        printf("  ❓ if 조건문 개수: %d개\n", local_if_count);
        if_count += local_if_count;

        // 다음 함수로 이동
        p += strlen("\"_nodetype\": \"FuncDef\"");
    }

    printf("\n✅ 총 함수 개수: %d개\n", func_count);
    printf("✅ 전체 if 조건문 개수: %d개\n", if_count);
}

int main() {
    FILE *fp = fopen("ast.json", "r");
    if (!fp) {
        perror("파일 열기 실패");
        return 1;
    }

    char *json_data = malloc(MAX_JSON_SIZE);
    if (!json_data) {
        printf("메모리 할당 실패\n");
        fclose(fp);
        return 1;
    }

    json_data[0] = '\0';
    char line[512];
    while (fgets(line, sizeof(line), fp)) {
        strcat(json_data, line);
    }
    fclose(fp);

    analyze_ast(json_data);
    free(json_data);
    return 0;
}
