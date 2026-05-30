DO $$
BEGIN
  IF NOT EXISTS (
    SELECT 1 
    FROM pg_type
    WHERE typname = 'role' 
  ) THEN 
    CREATE TYPE role AS ENUM('admin', 'user', guest);
  END IF;
END
$$

CREATE TYPE role AS ENUM ('admin', 'user', 'guest');

CREATE TABLE IF NOT EXISTS users (

	user_login VARCHAR(64) NOT NULL,
	user_password TEXT,
	user_role role NOT NULL

);