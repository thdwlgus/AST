#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_SIZE 100000

// 키워드 개수 세기 함수
int count_keyword(const char *src, const char *keyword) {
    int count = 0;
    const char *p = src;
    while ((p = strstr(p, keyword)) != NULL) {
        count++;
        p += strlen(keyword);
    }
    return count;
}

// "name": "something" 형태에서 이름 추출
const char* extract_string_value(const char *src, const char *key, char *buffer) {
    const char *p = strstr(src, key);
    if (!p) return NULL;
    p = strchr(p, '\"'); if (!p) return NULL;
    p = strchr(p + 1, '\"'); if (!p) return NULL;
    const char *start = p + 1;
    const char *end = strchr(start, '\"');
    if (!end) return NULL;
    int len = end - start;
    strncpy(buffer, start, len);
    buffer[len] = '\0';
    return end;
}

// "names": ["int"] 형태에서 타입 추출
const char* extract_type(const char *src, char *buffer) {
    const char *p = strstr(src, "\"names\"");
    if (!p) return NULL;
    const char *open = strchr(p, '[');
    const char *close = strchr(p, ']');
    if (!open || !close || close <= open) return NULL;
    int len = close - open - 1;
    strncpy(buffer, open + 1, len);
    buffer[len] = '\0';
    return close;
}

void analyze_ast(const char *json) {
    const char *p = json;
    int func_count = 0;
    int total_if_count = 0;

    printf("🔍 함수 분석 시작\n");

    while ((p = strstr(p, "\"_nodetype\": \"FuncDef\"")) != NULL) {
        func_count++;
        printf("\n🔹 [%d번째 함수]\n", func_count);

        // 함수 이름
        char func_name[100] = "(이름없음)";
        extract_string_value(p, "\"name\"", func_name);
        printf("  📛 이름: %s\n", func_name);

        // 리턴 타입
        char return_type[100] = "(타입없음)";
        extract_type(p, return_type);
        printf("  🔙 리턴타입: %s\n", return_type);

        // 파라미터 추출
        printf("  🛠️ 파라미터 목록:\n");
        const char *decl_ptr = p;
        while ((decl_ptr = strstr(decl_ptr, "\"_nodetype\": \"Decl\"")) != NULL) {
            // 함수 본문을 지나면 break
            if (strstr(p, "\"body\"") && decl_ptr > strstr(p, "\"body\"")) break;

            char param_name[100] = "(unnamed)";
            extract_string_value(decl_ptr, "\"name\"", param_name);

            char param_type[100] = "(type)";
            extract_type(decl_ptr, param_type);

            printf("    - %s: %s\n", param_name, param_type);
            decl_ptr += strlen("\"_nodetype\": \"Decl\"");
        }

        // if 조건문 개수
        int local_if = count_keyword(p, "\"_nodetype\": \"If\"");
        printf("  ❓ if 조건문: %d개\n", local_if);
        total_if_count += local_if;

        p += strlen("\"_nodetype\": \"FuncDef\"");
    }

    printf("\n✅ 총 함수 개수: %d개\n", func_count);
    printf("✅ 전체 if 조건문 개수: %d개\n", total_if_count);
}

int main() {
    FILE *fp = fopen("ast.json", "r");
    if (!fp) {
        printf("❌ ast.json 파일을 열 수 없습니다.\n");
        return 1;
    }

    char *json = malloc(MAX_SIZE);
    if (!json) {
        printf("❌ 메모리 할당 실패\n");
        fclose(fp);
        return 1;
    }

    json[0] = '\0';
    char line[1024];
    while (fgets(line, sizeof(line), fp)) {
        strcat(json, line);
    }
    fclose(fp);

    analyze_ast(json);

    free(json);
    return 0;
}
