#ifndef PHP_REGISTRY_H
#define PHP_REGISTRY_H


void UpdateIniFromRegistry(char *path, TSRMLS_D);
char *GetIniPathFromRegistry();

#endif /* PHP_REGISTRY_H */
