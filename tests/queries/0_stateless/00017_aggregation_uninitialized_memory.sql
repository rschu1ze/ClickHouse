-- Tags: stateful
SELECT DISTINCT (URLHierarchy(URL)[1]) AS q, 'x' AS w FROM test.hits WHERE CounterID = 14917930 ORDER BY URL

